#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <stdlib.h>
#include <vector>
using std::vector;

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);

	
void getTrainData(char * data[MAX_DATA_NUM], int data_num);

void getInputData(char * info[MAX_INFO_NUM]);

void printTrainData();
void printInputData();

void predictVM_ES();
void predictVM_test();
void dataDenoise();

void putFlavors2Server();

int getMonthSize(int month);

vector<double> getEs(vector<double> vec_data, double a, double s);

#endif
