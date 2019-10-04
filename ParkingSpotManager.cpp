// File:	ParkingSpotManager.cpp
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

#include "ParkingSpotManager.h"
#include <stdio.h>
#include <unordered_set>
#include "/usr/include/mysql/mysql.h"
#include <glog/logging.h>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <stdlib.h>
#include <errno.h>

#include <cstring>

int po_id[1000];
int tlimit[1000];


MYSQL *conn;
MYSQL_RES *res;
MYSQL_RES *ex_res;
MYSQL_ROW row;
MYSQL_ROW ex_row;


const char *server = "192.168.64.83";
//const char *server = "192.168.1.102"; //pocket-fi
//const char *server = "192.168.10.17";
//const char *server = "192.168.10.21"; //ethernet
//const char *server = "192.168.43.66";
const char *user = "root";
const char *password = "qisens123";
const char *database = "seevider";

using namespace std;

namespace seevider {
	ParkingSpotManager::ParkingSpotManager() {
		if (!readFromJSONFile(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS)) {
			LOG(WARNING) << "Cannot initialize parking spots from system file: " << SYSTEM_FILE_PARKINGSPOTS;
		}
	}

	ParkingSpotManager::~ParkingSpotManager() {
		boost::property_tree::write_json(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS, toPTree());
	}

	bool ParkingSpotManager::add(int id, std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy) {
		if (mParkingSpots.find(id) != mParkingSpots.cend()) {
			return false;
		}

		mParkingSpots[id] = std::make_shared<ParkingSpot>(id, spotName, timeLimit, roi, policy);

		return true;
	}

	bool ParkingSpotManager::update(int id, std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy) {
		iterator spot = mParkingSpots.find(id);
		if (spot == mParkingSpots.end()) {
			return false;
		}

		return spot->second->update(spotName, timeLimit, roi, policy);
	}

	bool ParkingSpotManager::erase(int id) {
		iterator spot = mParkingSpots.find(id);
		if (spot == mParkingSpots.end()) {
			return false;
		}

		spot->second->reset();
		mParkingSpots.erase(id);

		return true;
	}
//////////////////////////////////////////////MySQL
	void ParkingSpotManager::connectDB(){
		if( !(conn = mysql_init((MYSQL*)NULL))){
			printf("init fail\n");
		}

		
		if(!(mysql_real_connect(conn,server,user,password,database, 3306, NULL, 0))){
			printf("connect error.\n");
		}

		
		if(mysql_select_db(conn, database) !=0){
			mysql_close(conn);
			printf("select_db fail.\n");
		}
	}

	void ParkingSpotManager::createDB(int id, std::string spotName, int roi_x, int roi_y, int roi_h, int roi_w, int policy){
		//jeeeun //policy 5번으로 하드코딩
		policy = 5;

		connectDB();
		printf("start creating DB\n");
		
		stringstream query_str;
		query_str<<"insert into parking_spot (parking_spot_id, parking_spot_name, roi_coordx, roi_coordy, roi_height, roi_width, policy_id) values ("<<id<<", '"<<spotName<<"', "<<roi_x<<", "<<roi_y<<", "<<roi_h<<", "<<roi_w<<", "<<policy<<")";
		string query = query_str.str();
		cout<<query<<endl;
		const char* cd =query.c_str();
		cout<<cd<<endl;
		mysql_query(conn, cd);

		cout<<"Success creating DB"<<endl;
		mysql_close(conn);
	}
	bool ParkingSpotManager::isExist(int id){
		// id not exist return 0 -> createDB();
		// id exist return 1 -> updateDB();
		connectDB();
		printf("start searching DB\n");
		stringstream qq;
		qq<<"SELECT parking_spot_id FROM parking_spot WHERE parking_spot_id = "<< id <<"";
		string qry = qq.str();
		const char* ee = qry.c_str();
		mysql_query(conn,ee);
		ex_res = mysql_store_result(conn);
		if(mysql_fetch_row(ex_res)==0){
			mysql_close(conn);
			return false;
		}
		else{
			mysql_close(conn);
			return true;
		}

	}
	void ParkingSpotManager::updateDB(int id, std::string spotName, int roi_x, int roi_y, int roi_h, int roi_w, int policy) {
		connectDB();
		printf("start updateDB\n");
		string query;
		const char* q;

		//jeeeun //policy 5번으로 하드코딩
		policy = 5;
		
		stringstream str_query;
		str_query<<"UPDATE parking_spot SET parking_spot_name = '"<< spotName <<"', roi_coordx = '"<< roi_x <<"', roi_coordy = '"<< roi_y <<"', roi_height = '"<< roi_h <<"', roi_width = '"<< roi_w <<"', policy_id = '"<< policy <<"', WHERE parking_spot_id = "<< id <<"";	
		//jeeeun //policy_id 추가해줌. time_limit은 추가할필요 없음. policy 아이디 있으면 알아서 받아오니까.

		query = str_query.str();
		q = query.c_str();
		mysql_query(conn, q);
		
		printf("updateDB success.\n");

		mysql_close(conn);

	}


	void ParkingSpotManager::pullDB(){
		connectDB();
		if(mysql_query(conn,"SELECT s.parking_spot_id, s.policy_id, p.time_limit FROM parking_spot AS s LEFT JOIN parking_policy AS p ON s.policy_id = p.policy_id")){
			printf("query fail\n");
		}

		printf("query success\n");

		res = mysql_store_result(conn);
		
		while((row = mysql_fetch_row(res))!=NULL){ // from 1 to temp array save
			int temp;
			temp = atoi(row[0]);
			po_id[temp] = atoi(row[1]);
			tlimit[temp]=atoi(row[2]);

		}
		printf("pullDB success.\n");

		mysql_close(conn);
	}

/////void ParkingSpotManager::isRes_sensor(std::string sensorName) 나중에 sensorName 받아올수 있으면 이런 함수로 만들어서 구현
 	/*void ParkingSpotManager::isRes_sensor(){
		connectDB();
		if(mysql_query(conn,"SELECT reservation_sensor_yn FROM sensor WHERE sensor_id = 'qisens_side_s1'")){
			printf("query fail\n");
		}

		printf("sensor query success\n");

		res = mysql_store_result(conn);

		row = mysql_fetch_row(res);
		int temp_yn;
		temp_yn = atoi(row[0]);
		if(temp == 1)
			isReservation_sensor = true;
		else
			isReservation_sensor = false;

		mysql_close(conn);
	}


	bool ParkingSpotManager::isSensor(){
		return isReservation_sensor;
	}*/
//////////////////////////////////////////////////////////////////////////


	void ParkingSpotManager::clear() {
		reset();
		mParkingSpots.clear();
	}

	void ParkingSpotManager::reset() {
		for (auto &spot : mParkingSpots) {
			spot.second->reset();
		}
	}

	unsigned int ParkingSpotManager::size() const {
		return (unsigned int)mParkingSpots.size();
	}

	bool ParkingSpotManager::empty() const {
		return mParkingSpots.empty();
	}

	bool ParkingSpotManager::readFromJSONFile(std::string filename) {
		boost::property_tree::ptree ptree;
		pullDB();



		try {
			boost::property_tree::read_json(filename, ptree);
		}
		catch (const boost::property_tree::ptree_error &e) {
			LOG(ERROR) << e.what();
			return false;
		}

		for (auto &elem : ptree) {
			int id, timeLimit;
			std::string spotName;
			PARKING_SPOT_POLICY policy;
			cv::Rect roi;

			try {
				id = elem.second.get<int>("parkingSpotId");
				spotName = elem.second.get<std::string>("parkingSpotName");
				roi.x = elem.second.get<int>("roiCoordX");
				roi.y = elem.second.get<int>("roiCoordY");
				roi.width = elem.second.get<int>("roiWidth");
				roi.height = elem.second.get<int>("roiHeight");
				policy = (PARKING_SPOT_POLICY)po_id[id];
				timeLimit = tlimit[id];
				add(id, spotName, timeLimit, roi, policy);
				if(isExist(id))
                	updateDB(id, spotName, roi.x, roi.y, roi.height, roi.width, policy);
                else
					createDB(id, spotName, roi.x, roi.y, roi.height, roi.width, policy);
				printf("%d\n", id);
				cout<<spotName<<endl;
				printf("%d\n", roi.x);
				printf("%d\n", roi.y);
				printf("%d\n", roi.width);
				printf("%d\n", roi.height);
				printf("%d\n", policy);
				printf("%d\n\n\n", timeLimit);

			}
			catch (const boost::property_tree::ptree_error &e) {
				LOG(ERROR) << e.what();
			}
		}

		return true;
	}

	bool ParkingSpotManager::updateParkingSpots(boost::property_tree::ptree &root) {
		std::unordered_set<int> updated;

		for (auto elem : root) {
			int id, timeLimit;
			std::string spotName;
			PARKING_SPOT_POLICY policy;			
			cv::Rect roi;

			try {
				id = elem.second.get<int>("parkingSpotId");
				spotName = elem.second.get<std::string>("parkingSpotName");
				roi.x = elem.second.get<int>("roiCoordX");
				roi.y = elem.second.get<int>("roiCoordY");
				roi.width = elem.second.get<int>("roiWidth");
				roi.height = elem.second.get<int>("roiHeight");
				policy = (PARKING_SPOT_POLICY)po_id[id];
				timeLimit = tlimit[id];

				std::cout << id << ": " << spotName << ", " << roi << ", " << policy << ", " << timeLimit << std::endl;
			}
			catch (const boost::property_tree::ptree_error &e) {
				LOG(ERROR) << e.what();
			}

			auto ptr = mParkingSpots.find(id);
			if (ptr == mParkingSpots.end()) {
				add(id, spotName, timeLimit, roi, policy);
			}
			else {
				update(id, spotName, timeLimit, roi, policy);
			}

			updated.insert(id);
		}

		// Remove parking spots that are not in 'root'
		for (auto iter = mParkingSpots.begin(); iter != mParkingSpots.end();) {
			if (updated.find(iter->first) != updated.end()) {
				iter++;
			}
			else {
				iter = mParkingSpots.erase(iter);
			}
		}

		// Write json file
		boost::property_tree::write_json(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS, toPTree());

		return true;
	}

	boost::property_tree::ptree ParkingSpotManager::toPTree() const {
		boost::property_tree::ptree spotArray;

		for (auto elem : mParkingSpots) {
			std::shared_ptr<ParkingSpot> &spot = elem.second;
			boost::property_tree::ptree result;

			result.put("parkingSpotId", std::to_string(spot->ID));
			result.put("parkingSpotName", spot->SpotName);
			result.put("roiCoordX", spot->ROI.x);
			result.put("roiCoordY", spot->ROI.y);
			result.put("roiWidth", spot->ROI.width);
			result.put("roiHeight", spot->ROI.height);
			result.put("policyId", (int)spot->ParkingPolicy);
			result.put("timeLimit", spot->TimeLimit);

			spotArray.push_back(std::make_pair("", result));
		}

		return spotArray;
	}

	cv::Mat ParkingSpotManager::drawParkingStatus(cv::Mat frame) const {
		cv::Mat drawn = frame.clone();

		for (auto parkingSpot : mParkingSpots) {
			cv::Scalar color;
			std::string roi_text;
			cv::Scalar color_text = CV_RGB(0,0,0); //black

			if(parkingSpot.second->isOverstayed()){
				color = CV_RGB(255,255,0);
				roi_text = "Overstay";
			}
			else if (parkingSpot.second->isOccupied()) {
				color = CV_RGB(255, 0, 0);
				roi_text = "Occupied";
			}
			else {
				color = CV_RGB(0, 255, 0);
				roi_text = "";	//available
			}

			cv::rectangle(drawn, parkingSpot.second->ROI, color, 2);

			//insert text	//jeeeun_putText
			cv::putText(drawn, roi_text, cv::Point(parkingSpot.second->ROI.x, parkingSpot.second->ROI.y), cv::FONT_HERSHEY_PLAIN, 2.0, color, 2);
			cv::putText(drawn, parkingSpot.second->mPlateNumber, cv::Point(parkingSpot.second->ROI.x, (parkingSpot.second->ROI.y + parkingSpot.second->ROI.height + 20)), cv::FONT_HERSHEY_PLAIN, 2.0, color_text, 2);
			//std::cout << "plate number in ParkingSpotmanager.cpp : " << parkingSpot.second->mPlateNumber << std::endl;

		}

		return drawn;
	}

	ParkingSpotManager::iterator ParkingSpotManager::begin() {
		return mParkingSpots.begin();
	}

	ParkingSpotManager::const_iterator ParkingSpotManager::begin() const {
		return mParkingSpots.cbegin();
	}

	ParkingSpotManager::iterator ParkingSpotManager::end() {
		return mParkingSpots.end();
	}

	ParkingSpotManager::const_iterator ParkingSpotManager::end() const {
		return mParkingSpots.cend();
	}

	std::shared_ptr<ParkingSpot> ParkingSpotManager::operator[](const int id) {
		return mParkingSpots[id];
	}

	const std::shared_ptr<ParkingSpot> ParkingSpotManager::operator[](const int id) const {
		const_iterator spot = mParkingSpots.find(id);

		if (spot != mParkingSpots.cend()) {
			return spot->second;
		}
		else {
			return nullptr;
		}
	}
}
