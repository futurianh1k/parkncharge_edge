// File:	ParkingSpot.cpp
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017
//

#include "ParkingSpot.h"
#include "types.h"
#include "ParkingUpdateMessage.h"
#include <glog/logging.h>
#include <stdio.h>
#include "/usr/include/mysql/mysql.h"
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <sstream>
#include <math.h>
#include <string.h>

using namespace seevider;
using namespace std;
namespace pt = boost::posix_time;

using cv::Mat;

int seq[1000];
int cnt_now;
bool check_seq[1000];
int s_date, e_date, s_time, e_time, s_y, s_mo, s_d, s_h, s_m, e_y, e_mo;
//FROM DB ABOUT RESERVATRION START DATE, TIME, END DATE, TIME (STRING) ->>>> YEAR, MONTH, DAY, HOUR, MINUTES (INT)


long long e_d, e_h, e_m; // Vehicle enter time

long long yy, yyq, mo, moq, dd, ddq, mm, mmq, hh, hhq; //enter time (STRING) ->>> YEAR, MONTH, DAY, HOUR, MINUTES (INT)
int sub_m[1000], sub_d[1000], sub_h[1000];
bool reser_status[1000];

MYSQL *con_spot;
MYSQL_RES *res_spot;
MYSQL_ROW row_spot;


char *serve = "35.164.140.47";
char *use = "seevider";
char *passwor = "seevider123";
char *databas = "seevider";


ParkingSpot::ParkingSpot(int id, std::string spotName, const int length,
	const cv::Rect roi, PARKING_SPOT_POLICY policy) : ID(mID), SpotName(mSpotName),
	ROI(mROI), TimeLimit(mTimeLimit), ParkingPolicy(mParkingPolicy),
	UpdateEnabled(mUpdateEnabled),
	mID(id), mSpotName(spotName), mTimeLimit(length), mROI(roi),
	mParkingPolicy(policy), mOccupied(false), mOverstayed(false), mPlateNumber("null"),
	mUpdateEnabled(true),
	mWork(mService), mTimerThread(boost::bind(&ParkingSpot::runTimer, this)),
	mParkingTimer(mService) {
}

ParkingSpot::~ParkingSpot() {
    mService.stop();
    mTimerThread.join();

	DLOG(INFO) << "Release parking spot " << mID;
}

bool ParkingSpot::isOccupied() const {
    return mOccupied;
}

bool ParkingSpot::isOverstayed() const {
	return mOccupied && mOverstayed;
}

///////////////////////////////////////////

void ParkingSpot::connect_DB(){ // MySQL DB CONNECT
	if( !(con_spot = mysql_init((MYSQL*)NULL))){
		printf("init fail\n");
		
	}

	//printf("mysql_init success.\n");

	if(!(mysql_real_connect(con_spot,serve,use,passwor,databas, 3306, NULL, 0))){
		printf("connect error.\n");
		
	}

	//printf("mysql_real_connect suc.\n");

	if(mysql_select_db(con_spot, databas) !=0){
		mysql_close(con_spot);
		printf("select_db fail.\n");
		
	}
	//printf("connect success.\n");
}

int ParkingSpot::entVehicle(const std::string PN, const std::string entryTime){ 
	
	connect_DB();
	
	stringstream ev;
	string yye, moe, dde, hhe, mme;
	
	ev<<"SELECT sequence, start_date, end_date, start_time, end_time, plate_number FROM end_user_reservation WHERE plate_number = '"<< PN <<"' AND used_yn = 'N'";
	string qye = ev.str();
	

	const char* evv =qye.c_str();

	
	
	mysql_query(con_spot, evv);
	
	res_spot = mysql_store_result(con_spot);
	
	row_spot = mysql_fetch_row(res_spot); //////////
	int tmp;
	
	tmp = atoi(row_spot[0]);
	string updateQy;
	const char* upqy;
	stringstream upq;
	upq<<"UPDATE end_user_reservation SET used_yn = 'Y' WHERE sequence = "<<tmp<<"";
	updateQy = upq.str();
	upqy = updateQy.c_str();
	mysql_query(con_spot,upqy);
	LOG(INFO) << tmp;
	cout<<entryTime<<endl;
	for(int i=0; i<=12; i++)
	{
		if(i<=3)
			yye+=entryTime[i];
		else if(i<=5)
			moe+=entryTime[i];
		else if(i<=7)
			dde+=entryTime[i];
		else if(i>8 && i<=10)
			hhe+=entryTime[i];
		else if(i>10)
			mme+=entryTime[i];
	}
	yyq = atoi(yye.c_str());
	moq = atoi(moe.c_str());
	ddq = atoi(dde.c_str()) * 86400;
	hhq = atoi(hhe.c_str()) * 3600;
	mmq = atoi(mme.c_str()) * 60;
	long long ent_sum = ddq + hhq + mmq;
	
	if(check_seq[tmp]==false)
		check_seq[tmp] = true;
	s_date=atoi(row_spot[1]); // start_date
	s_time=atoi(row_spot[3]); // start_time
	e_date=atoi(row_spot[2]); // end_date
	e_time=atoi(row_spot[4]); //end_time

	e_y = e_date / 10000;
	e_mo = (e_date % 10000) / 100;
	e_d = (e_date % 10000) % 100 * 86400;
	e_h = e_time / 100 * 3600;
	e_m = e_time % 100 * 60;
	
	long long db_sum = e_d + e_h + e_m;
	cout<<db_sum<<endl;
	cout<<ent_sum<<endl;

	LOG(INFO) << e_d<<" : "<<e_h<<" : "<<e_m;
	LOG(INFO) << ddq<<" : "<<hhq<<" : "<<mmq;

	int result_timer = 0;
	
	if(db_sum - ent_sum>0)
		result_timer = db_sum - ent_sum;
	else
		result_timer = 10;
	
	LOG(INFO) << "result_timer : "<< result_timer;
	

	mysql_close(con_spot);

	return result_timer;
}

void ParkingSpot::extVehicle(const std::string PN, const std::string entryTime){

// 나가는 시간이 들어옴!! entryTime!
// 그래서 mysql 시간과 비교해서 overtime을 알려줄꺼임 

	// 0: occupied, 1: overtime
	connect_DB();
	stringstream ee;
	string yyy, mom, ddd, hhh, mmm;

	ee<<"SELECT sequence, start_date, end_date, start_time, end_time, facility_sequence, plate_number FROM end_user_reservation WHERE plate_number = '"<< PN <<"' ORDER BY start_date, start_time"; // + WHERE delete_yn = N !!!!!! THIS IS TEST!!!
	string qy = ee.str();
	
	const char* ev =qy.c_str();
	
	mysql_query(con_spot, ev);
	res_spot = mysql_store_result(con_spot);
	row_spot = mysql_fetch_row(res_spot);
	int temp;
	
	temp = atoi(row_spot[0]);

	
	for(int i=0; i<=12; i++)
	{
		if(i<=3)
			yyy+=entryTime[i];
		else if(i<=5)
			mom+=entryTime[i];
		else if(i<=7)
			ddd+=entryTime[i];
		else if(i>8 && i<=10)
			hhh+=entryTime[i];
		else if(i>10)
			mmm+=entryTime[i];
	}

/////// exit 한 시간을 세분화 시킴
	
	yy =atoi(yyy.c_str());
	mo =atoi(mom.c_str());
	dd =atoi(ddd.c_str());
	hh =atoi(hhh.c_str());
	mm =atoi(mmm.c_str());


	if(check_seq[temp]==false)
		check_seq[temp] = true;
	s_date =atoi(row_spot[1]); // start_date
	s_time =atoi(row_spot[3]); // start_time
	e_date =atoi(row_spot[2]); // end_date
	e_time =atoi(row_spot[4]); //end_time

////////////////////////reservation time actually this part is already exist/
	e_y = e_date / 10000;
	e_mo = (e_date % 10000) / 100;
	e_d = (e_date % 10000) % 100;
	e_h = e_time / 100;
	e_m = e_time % 100;

	sub_d[temp] = e_d - dd; // reservation day - exit day
	sub_h[temp] = e_h - hh; // reservation hour - exit hour
	sub_m[temp] = e_m - mm; // reservation minutes - exit minutes
/*
printf("\n\n");
printf("\n\n");
printf("%d %d %d",e_d[temp],e_h[temp], e_m[temp]);
printf("\n\n");
printf("%d %d %d",dd[temp], hh[temp], mm[temp]);
printf("\n\n");
printf("\n\n");
printf("%d %d %d",sub_d[temp], sub_h[temp], sub_m[temp]);*/
//  reser_status = 0; false: occupied, true: overtime

	if(sub_d[temp]<0)
		reser_status[temp] = true;

	if(sub_h[temp]<0){
		reser_status[temp] = true;
		sub_h[temp] = abs(sub_h[temp]);
	}

	if(sub_m[temp]<0){
		reser_status[temp] = true;
		sub_m[temp] = abs(sub_m[temp]);
	}
/*
	if(reser_status[temp]==true)
	{
		printf("\n\ntest:::::-----------------------------------(overtime)\n\n");
	}
	else
	{
		printf("\n\ntest:::::-----------------------------------(normal)\n\n");
	}

*/
	//plate_number[temp]=row_spot[6];
	cnt_now = temp;
	mysql_close(con_spot);


}




///////////////////////////////////////////////////////

void ParkingSpot::enter(const Mat& entryImage, const cv::Rect &ROI, const pt::ptime &entryTime, const std::string PN) {
	Mat cropImage;
	if (ROI.width <= entryImage.rows && ROI.height <= entryImage.cols){
		cropImage = entryImage(ROI);
	}
	else {
		cropImage = entryImage;
	}
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_ENTER, mID, entryImage, cropImage, entryTime, PN);
	mOccupied = true;
	mPlateNumber = PN;
	int res_timer; // ent_vehicle's timeLimit
	
	res_timer = entVehicle(PN, to_iso_string(entryTime));
	LOG(INFO) << "res_timer :  " <<res_timer;
    mServerMsgQueue->push(data);
	mEntryTime = entryTime;
	startTimer(res_timer); 
	// startTimer();
	LOG(INFO) << "Parking spot ID " << mID << " has occupied at " << to_iso_string(entryTime) + " by " + PN;
}

void ParkingSpot::overstayed(const Mat& expiredImage, const pt::ptime &exprTime) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_OVER, mID, expiredImage.clone(), exprTime);
	mOverstayed = true;
	mServerMsgQueue->push(data);



}

void ParkingSpot::exit(const Mat& exitImage, const pt::ptime &exitTime, const std::string PN) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_EXIT, mID, exitImage, exitTime);
    mOccupied = false;
	mOverstayed = false;
	mPlateNumber = "null";
	mServerMsgQueue->push(data);
	stopTimer();
	LOG(INFO) << "Parking spot ID " << mID << " has released at " << to_simple_string(exitTime);

	extVehicle(PN, to_iso_string(exitTime));


}

bool ParkingSpot::update(std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy) {
	if (mOccupied && !mOverstayed) {
		LOG(WARNING) << "Failed to update parking spot id " << mID << ". To update a parking spot, parking timer must be disabled.";
		return false;
	}

	mSpotName = spotName;
	mTimeLimit = timeLimit;
	mROI = roi;
	mParkingPolicy = policy;

	return true;
}

bool ParkingSpot::update(bool occupied, bool triggerUpdatability) {
	bool updated = false;
	if (occupied) {
		if (mOccupiedFrameCounter < mPositiveThreshold) {
			mOccupiedFrameCounter++;
			//cout<<"test1: "<<mOccupiedFrameCounter;
			LOG(INFO) << "(TEST!!)"<<mOccupiedFrameCounter;
		}
		else if (mOccupiedFrameCounter - mPositiveThreshold < mPositiveThreshold) {
			LOG(INFO) << "(TEST##)"<<mOccupiedFrameCounter;
			if (!mOccupied) {
				
				mOccupied = true;
				updated = true;
			}

			mOccupiedFrameCounter++;
		}
		else if (mOccupiedFrameCounter - mPositiveThreshold >= mPositiveThreshold && triggerUpdatability) {
			mUpdateEnabled = false;
		}
	}
	else {
		if (mOccupiedFrameCounter > mNegativeThreshold) {
			mOccupiedFrameCounter--;
		}
		else if (mOccupiedFrameCounter - mNegativeThreshold > mNegativeThreshold) {
			if (mOccupied) {
				mOccupied = false;
				updated = true;
			}

			mOccupiedFrameCounter--;
		}
		else if (mOccupiedFrameCounter - mNegativeThreshold <= mNegativeThreshold && triggerUpdatability) {
			mUpdateEnabled = false;
		}
	}

	return updated;
}

void ParkingSpot::reset() {
	stopTimer();
	mOccupied = false;
	mOverstayed = false;
	mUpdateEnabled = true;
	mOccupiedFrameCounter = 0;
}

void ParkingSpot::runTimer() {
    mService.run();
}

void ParkingSpot::startTimer(int res_timeLimit) {
    printf("\n\nstartTimer!! : %d\n\n", res_timeLimit); 
    mParkingTimer.expires_from_now(boost::posix_time::seconds(res_timeLimit));
    
    mTimeLimit = res_timeLimit;
    mParkingTimer.async_wait(boost::bind(&ParkingSpot::notifyExpiration, this));   
}

void ParkingSpot::stopTimer() {
    mParkingTimer.cancel();
}

void ParkingSpot::notifyExpiration() {
    if (mOccupied) {
	LOG(INFO) << mTimeLimit;
	//setTimeLimit(res_timeLimit);
        Mat frame;
		pt::ptime time = mEntryTime + boost::posix_time::seconds(mTimeLimit);
	LOG(INFO) <<time;
        mVideoReader->readAt(frame, time);

		LOG(INFO) << "Parking spot ID " << mID << " has expired.";
		overstayed(frame, time);
    }
}

//-----------------------------------
// Static functions
//-----------------------------------

void ParkingSpot::setMessageQueue(std::shared_ptr<MessageQueue> &messageQueue) {
    mServerMsgQueue = messageQueue;
}

void ParkingSpot::setVideoReader(std::shared_ptr<SerialVideoReader> &videoReader) {
    mVideoReader = videoReader;
}

void ParkingSpot::setPositiveThreshold(int positiveThreshold) {
	mPositiveThreshold = positiveThreshold;
}

void ParkingSpot::setNegativeThreshold(int negativeThreshold) {
	mNegativeThreshold = negativeThreshold;
}

//-----------------------------------
// Static variables
//-----------------------------------

std::shared_ptr<MessageQueue> ParkingSpot::mServerMsgQueue = nullptr;
std::shared_ptr<SerialVideoReader> ParkingSpot::mVideoReader = nullptr;
int ParkingSpot::mPositiveThreshold = 5;
int ParkingSpot::mNegativeThreshold = -3;
