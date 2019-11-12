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
#include "ParkingSpotManager.h"
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

//#include <curl/curl.h>

using namespace seevider;
using namespace std;
namespace pt = boost::posix_time;

using cv::Mat;

int seq[1000];
int cnt_now;
bool check_seq[1000];
int start_date, end_date, start_time, end_time, start_year, start_mon, start_day, start_hour, start_min;
//FROM DB ABOUT RESERVATRION START DATE, TIME, END DATE, TIME (STRING) ->>>> YEAR, MONTH, DAY, HOUR, MINUTES (INT)


long long ent_year, ent_mon, ent_day, ent_hour, ent_minutes; // Vehicle enter time

long long year_total, mon_total, day_total, hour_total, min_total; //enter time (STRING) ->>> YEAR, MONTH, DAY, HOUR, MINUTES (INT)

MYSQL *con_spot;
MYSQL_RES *res_spot;
MYSQL_ROW row_spot;
MYSQL *con_check;
MYSQL_RES *res_check;
MYSQL_ROW row_check;

std::string url_docker_server = "192.168.60.14:51000/api/lpr";

const char *serve = "192.168.64.83";
//const char *serve = "192.168.43.66";
const char *use = "root";
const char *passwor = "qisens123";
const char *databas = "seevider";

bool isWork;
bool isReservation_sensor;

ParkingSpot::ParkingSpot(int id, std::string spotName, const int length,
	const cv::Rect roi, PARKING_SPOT_POLICY policy) : ID(mID), SpotName(mSpotName),
	ROI(mROI), TimeLimit(mTimeLimit), ParkingPolicy(mParkingPolicy),
	UpdateEnabled(mUpdateEnabled),
	mID(id), mSpotName(spotName), mTimeLimit(length), mROI(roi),
	mParkingPolicy(policy), mOccupied(false), mOverstayed(false),
	mUpdateEnabled(true),
	mWork(mService), mTimerThread(boost::bind(&ParkingSpot::runTimer, this)),
	mParkingTimer(mService) {
}

ParkingSpot::~ParkingSpot() {
    mService.stop();
    mTimerThread.join();

	DLOG(INFO) << "Release parking spot " << mID;
}

//jeeeun-----------------------------------------------
void ParkingSpot::setPlateNumber(std::string plate) {
        mPlateNumber = plate;
}

std::string ParkingSpot::getPlateNumber() {
        return mPlateNumber;
}


void ParkingSpot::setCarBrand(std::string brand) {
	mCarBrand = brand;
}

std::string ParkingSpot::getCarBrand() {
	return mCarBrand;
}
//-----------------------------------------------------

bool ParkingSpot::isOccupied() const {
    return mOccupied;
}

bool ParkingSpot::isOverstayed() const {
	return mOccupied && mOverstayed;
}
//-------------juhee
void ParkingSpot::setLocalizerROI(cv::Rect roiP)
{
	ParkingSpot::pROI = roiP;
}
void ParkingSpot::setVehicleROI(cv::Rect roiV)
{
	ParkingSpot::vROI = roiV;
}
cv::Rect ParkingSpot::getLocalizerROI()
{
	return pROI;
}
cv::Rect ParkingSpot::getVehicleROI()
{
	return vROI;
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
	
	ev<<"SELECT sequence, start_date, end_date, start_time, end_time, plate_number FROM end_user_reservation WHERE plate_number = '"<< PN <<"' AND used_yn = 'N' ORDER BY start_date, start_time";
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
	year_total = atoi(yye.c_str()) * 15768000;
	mon_total = atoi(moe.c_str()) * 43200;
	day_total = atoi(dde.c_str()) * 1440;
	hour_total = atoi(hhe.c_str()) * 60;
	min_total = atoi(mme.c_str()) ;
	long long ent_sum = year_total + mon_total + day_total + hour_total + min_total;
	
	start_date=atoi(row_spot[1]); // start_date
	start_time=atoi(row_spot[3]); // start_time
	end_date=atoi(row_spot[2]); // end_date
	end_time=atoi(row_spot[4]); //end_time

	ent_year = end_date / 10000 * 15768000;
	ent_mon = (end_date % 10000) / 100 * 43200;
	ent_day = (end_date % 10000) % 100 * 1440;
	ent_hour = end_time / 100 * 60;
	ent_minutes = end_time % 100;
	
	long long db_sum = ent_year + ent_mon + ent_day + ent_hour + ent_minutes;

	cout<<db_sum<<endl;
	cout<<ent_sum<<endl;

	LOG(INFO) << ent_year << ent_mon << ent_day << " : " << ent_hour << " : " << ent_minutes;
	LOG(INFO) << year_total << mon_total << day_total << " : " << hour_total << " : " << min_total;

	int result_timer = 0;
	
	if(db_sum - ent_sum>0)
		result_timer = db_sum - ent_sum;
	else
		result_timer = 10;
	
	LOG(INFO) << "result_timer : "<< result_timer;
	

	mysql_close(con_spot);

	return result_timer;
}


bool ParkingSpot::check_res(const std::string PN){
	stringstream cr;
	cr<<"SELECT sequence FROM end_user_reservation WHERE plate_number = '"<<PN<<"'";
	string qqq = cr.str();
	const char* eee =qqq.c_str();

	if( !(con_check = mysql_init((MYSQL*)NULL))){
		printf("init fail\n");
	}

	if(!(mysql_real_connect(con_check, serve, use, passwor, databas, 3306, NULL, 0))){
		printf("connect error.\n");
	}
	mysql_query(con_check, eee);

	res_check = mysql_store_result(con_check);
	row_check = mysql_fetch_row(res_check);
 
	unsigned long *lenths = mysql_fetch_lengths(res_check);
	mysql_close(con_check);
	if(lenths == NULL){
		return false;
	}
	else
		return true;
	
}

void ParkingSpot::enter(const Mat& entryImage, const cv::Rect &ROI, const pt::ptime &entryTime, const std::string PN) {
/*
	if(isWork==false)
		isRes_sensor();
	isWork = true;
*/
	Mat cropImage;
	if (ROI.width <= entryImage.rows && ROI.height <= entryImage.cols){
		cropImage = entryImage(ROI);
	}
	else {
		cropImage = entryImage;
	}
//----------------------------------------------------------------------------------------------------                    	
	//show crop image	//jeeeun
//	cv::imshow("cropImage", cropImage);
//	cv::waitKey(0);

//----------------------------------------------------------------------------------------------------                    
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_ENTER, mID, entryImage, cropImage, entryTime, PN);
	mOccupied = true;
	//mPlateNumber = PN;
	int res_timer; // ent_vehicle's timeLimit

	if(!isSensor()) //   normal spot's sensor
	{
		startTimer();
	}
	else if(check_res(PN)) // reservation sensor. check plate number
	{
		res_timer = entVehicle(PN, to_iso_string(entryTime));
		LOG(INFO) << "res_timer :  " <<res_timer;
		startTimer(res_timer, PN);
	}
	else {
		startTimer();
	}
	 // normal user
		
	/*
	if(check_res(PN)) // reservation user
	{
		res_timer = entVehicle(PN, to_iso_string(entryTime));
		LOG(INFO) << "res_timer :  " <<res_timer;
		startTimer(res_timer, PN);
	}
	else // normal user
		startTimer();
	*/

    	mServerMsgQueue->push(data);
	mEntryTime = entryTime;
	 
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
	mPlateNumber.clear();
	setLocalizerROI(cv::Rect (0,0,0,0));
	setVehicleROI(cv::Rect(0,0,0,0));
	mServerMsgQueue->push(data);
	stopTimer();
	LOG(INFO) << "Parking spot ID " << mID << " has released at " << to_simple_string(exitTime);
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

bool ParkingSpot::update(bool occupied) {
	bool updated = false;
	if (occupied) {
		if (mOccupiedFrameCounter < mPositiveThreshold) {
			mOccupiedFrameCounter++;
		}
		else if (mOccupiedFrameCounter - mPositiveThreshold < mPositiveThreshold) {
			if (!mOccupied) {
				
				mOccupied = true;
				updated = true;
			}

			mOccupiedFrameCounter++;
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

/////void ParkingSpotManager::isRes_sensor(std::string sensorName) 나중에 sensorName 받아올수 있으면 이런 함수로 만들어서 구현
void ParkingSpot::isRes_sensor(){
	connect_DB();
	if(mysql_query(con_spot,"SELECT reservation_sensor_yn FROM sensor WHERE sensor_id = 'qisens_side_s1'")){
		printf("query fail\n");
	}

	printf("sensor query success\n");

	res_spot = mysql_store_result(con_spot);

	row_spot = mysql_fetch_row(res_spot);
	int temp_yn;
	temp_yn = atoi(row_spot[0]);
	if(temp_yn == 1)
		isReservation_sensor = true;
	else
		isReservation_sensor = false;

	mysql_close(con_spot);
}


bool ParkingSpot::isSensor(){
	return isReservation_sensor;
}

void ParkingSpot::runTimer() {
    mService.run();
}

void ParkingSpot::startTimer(int res_timeLimit, const std::string PN) {
	
	printf("reservation user\n");
	mParkingTimer.expires_from_now(boost::posix_time::minutes(res_timeLimit));
	mTimeLimit = res_timeLimit;
	mParkingTimer.async_wait(boost::bind(&ParkingSpot::notifyExpiration, this));
    

}

void ParkingSpot::startTimer() {
    	printf("normal user\n");
	mParkingTimer.expires_from_now(boost::posix_time::seconds(mTimeLimit));
	mParkingTimer.async_wait(boost::bind(&ParkingSpot::notifyExpiration, this));
}

void ParkingSpot::stopTimer() {
    mParkingTimer.cancel();
}

void ParkingSpot::notifyExpiration() {
    if (mOccupied) {
	LOG(INFO) << mTimeLimit;
        Mat frame;
	pt::ptime time = mEntryTime + boost::posix_time::minutes(mTimeLimit);
	LOG(INFO) <<time;
        
	printf("chkpoint 1\n");
	LOG(INFO) << "Check VideoReader operation";
	LOG(INFO) << "Time in Minutes " << time;

        //read at "Time"
	time = mEntryTime + boost::posix_time::seconds(mTimeLimit);
	LOG(INFO) << "Time in Seconds " << time;
        mVideoReader->readAt(frame, time);
        //if(frame.empty()){
             //Ken trial: read again
        //     mVideoReader->readAt(frame, time);
        //}


	printf("chkpoint 2\n");
	LOG(INFO) << "Check VideoReader operation";
        LOG(INFO) << "Parking spot ID " << mID << " has expired.";
	LOG(INFO) << "Time in Minutes " << time;
	time = mEntryTime + boost::posix_time::minutes(mTimeLimit);
	overstayed(frame, time);

	cv::Scalar black;
	black = CV_RGB(0,0,0);
	//jeeeun add puttext spot id
	//cv::putText(frame, mID, cv::Point(ROI.x, ROI.y), cv::FONT_HERSHEY_PLAIN, 2.0, black, 2);

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
