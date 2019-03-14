#include "predict.h"

#include <stdlib.h>
#include <vector>
#include <cstring>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <math.h>

using std::string;
using std::vector;
using namespace std;

//trainData�е������ͳ����Ϣ
struct Flavor
{
    int count[13][32];
    int year;
};

//inputData�е��������Ϣ
struct FlavorInfo
{
    int number;     //����������
    int needCpuNum; //������������Ҫ��cpu����
    int needMemNum; //������������Ҫ���ڴ��С

    int predictNum; //�������Ԥ�����������
    int putNum;     //����������������
};

//inputData�еķ�������Ϣ
struct ServerInfo
{

    string name; //��������������

    int cpuNum;  //cpu����
    int memNum;  //�ڴ��С
    int diskNum; //Ӳ������

    int freeCpuNum; //����ʱʣ���cpu����
    int freeMemNum; //����ʱʣ����ڴ��С

    vector<FlavorInfo> flavors; //��ҪԤ����������Ϣ

    int needNum; //�����������Ҫʹ�ø÷�����������
};

//inputData��Ԥ��ʱ�����Ϣ
struct Period
{
    int startDate[3]; //����Ԫ�طֱ��ʾ:��,��,��
    int endDate[3];
};

//inputData
struct InputData
{
    int serverCategoryNum; //��������������

    ServerInfo serverInfo[4]; //�������͵ķ�����

    int flavorNum; //��ҪԤ���������������

    std::vector<FlavorInfo> flavors; //ÿ����������ľ�����Ϣ

    Period period;       //��ҪԤ���ʱ���
    int predictDayCount; //Ԥ��ʱ��ε�������
};

//ѵ�����ݼ�������������Ϣ
Flavor flavor[19];
int trainEndMonth; //��¼ѵ�����ݼ��Ľ�������
int trainEndDay;
int trainDayCount; //ͳ��ѵ�����ݼ��ܹ��ж�����

//����ѵ������Ԥ��ʱ���֮���ʱ������
int train2Predict_days;

//input������Ϣ
InputData inputData;

//��ÿ�������Ԥ��Ľ��
vector<FlavorInfo> predictFlavors;
int predictFlavorsCount; //Ԥ������������������

//����������Ľ��
std::vector<ServerInfo> res_servers;
std::vector<ServerInfo> putServers[3];
int needNum[3];

//ģ���˻�����Ҫ�õ��Ľṹ�����
struct Best_Result
{
    double minNum;
    vector<ServerInfo> best_servers;
    vector<FlavorInfo> best_vec_flavors;
};
Best_Result best_Result;

//��Ҫ��ɵĹ��������
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM], int data_num, char *filename)
{

    //��ȡѵ������
    getTrainData(data, data_num);
    //��ӡѵ������
    printTrainData();

    //��ȡinput����
    getInputData(info);
    //��ӡ��input������Ϣ
    printInputData();

    //�����ݽ���һ����ȥ��
    dataDenoise();

    //Ԥ�����������
    //predictVM_ES();
    predictVM_test();

    //��Ԥ���������������䵽��������
    putFlavors2Server();

    //��Ԥ�����Լ����������浽�ļ�
    string result_str = "";

    std::ostringstream ostr; //�����ַ����������ַ����Ķ�ȡ�뱣��

    //1.��Ԥ��������
    ostr << predictFlavorsCount; //��Ԥ�����������������浽result_str��
    ostr << "\n";
    result_str += ostr.str();

    for (int i = 0; i < inputData.flavorNum; ++i)
    {

        ostr.str(""); //����ַ�����

        ostr << "flavor";
        ostr << predictFlavors[i].number;
        ostr << " ";
        ostr << predictFlavors[i].predictNum;
        ostr << "\n";

        result_str += ostr.str();
    }

    //2.������������
    ostr.str("");
    ostr << "\n";
    for (int i = 0; i < 3; ++i)
    {

        if (i == 0 && needNum[0] != 0)
        {
            ostr << "General";
            ostr << " ";
            ostr << needNum[0];
            ostr << "\n";

            int len = 0;
            for (auto server : putServers[0])
            {
                ostr << "General-";
                ostr << (len + 1);
                ostr << " ";

                for (auto flavor : server.flavors)
                {
                    ostr << "flavor";
                    ostr << flavor.number;
                    ostr << " ";
                    ostr << flavor.putNum;
                    ostr << " ";
                }

                ostr << "\n";
                ++len;
            }
            ostr << "\n";
        } //if

        if (i == 1 && needNum[1] != 0)
        {
            //ostr<<"\n";
            ostr << "Large-Memory";
            ostr << " ";
            ostr << needNum[1];
            ostr << "\n";

            int len = 0;
            for (auto server : putServers[1])
            {
                ostr << "Large-Memory-";
                ostr << (len + 1);
                ostr << " ";

                for (auto flavor : server.flavors)
                {
                    ostr << "flavor";
                    ostr << flavor.number;
                    ostr << " ";
                    ostr << flavor.putNum;
                    ostr << " ";
                }
                ostr << "\n";
                ++len;
            }
            ostr << "\n";
        } //if

        if (i == 2 && needNum[2] != 0)
        {
            //ostr<<"\n";
            ostr << "High-Performance";
            ostr << " ";
            ostr << needNum[2];
            ostr << "\n";

            int len = 0;
            for (auto server : putServers[2])
            {
                ostr << "High-Performance-";
                ostr << (len + 1);
                ostr << " ";

                for (auto flavor : server.flavors)
                {
                    ostr << "flavor";
                    ostr << flavor.number;
                    ostr << " ";
                    ostr << flavor.putNum;
                    ostr << " ";
                }
                ostr << "\n";
                ++len;
            }
            //ostr<<"\n";
        } //if

    } //for

    result_str += ostr.str();
    char str[500000];
    char *result_file;

    strcpy(str, result_str.c_str());

    result_file = str;

    std::cout << "=============================The last result===========================" << std::endl;
    // ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ�ԣ�����н⣬��һ��ֻ��һ�����ݣ��ڶ���Ϊ�գ������п�ʼ���Ǿ�������ݣ�����֮����һ���ո�ָ���)
    write_result(result_file, filename);
    std::cout << "=============================The last result===========================" << std::endl;
}
//1.���ı��л�ȡѵ������
void getTrainData(char *data[MAX_DATA_NUM], int data_num)
{
    std::cout<<"data_line_num:"<<data_num<<std::endl;
    for(int i = 0; i < data_num; ++i){

        const char *trainInfo = data[i];
        if(trainInfo != NULL){

            string trainData(trainInfo);
            std::istringstream ss(trainData);

            string trainDataUnit;
           
            int flaNum = 0;
            int month = 0, day = 0;
            for(int n = 0; n < 4; ++n){
                 
                ss >> trainDataUnit;
                //��ȡ����С
                if(n == 1){
                    
                    if(trainDataUnit.length() == 7)
                        flaNum = int(trainDataUnit[6] - '0');    
                    else
                        flaNum = 10*int(trainDataUnit[6] - '0') + int(trainDataUnit[7] - '0');   
                }
                //��ȡʱ��
                if(n == 2){
                    month = 10*int(trainDataUnit[5] - '0') + int(trainDataUnit[6] - '0');
                    day = 10*int(trainDataUnit[8] - '0') + int(trainDataUnit[9] - '0');
                }

            }//for
            
            //���ݹ���С������Ӧ��ʱ�������Ӧ��ͳ��
            if(flaNum <= 18)
                flavor[flaNum].count[month][day] += 1;

            //��¼ѵ�����ݼ��Ľ�������
            trainEndMonth = month;
            trainEndDay = day;
        }//if
            
    }//for
}

//2.��ȡinput�ļ��е�������Ϣ
void getInputData(char *info[MAX_INFO_NUM])
{

    int index = 0;
    int flavorNum = 0;

    int serverCategoryNum = 5;        //��������������
    bool startReadServerInfo = false; //��ȡ��������Ϣ�Ŀ���

    while (true)
    {
        const char *inputInfo = info[index];

        if (inputInfo != NULL)
        {

            string infoData(inputInfo);
            std::istringstream ss(infoData);

            if (index == 0)
            {
                ss >> inputData.serverCategoryNum;
                serverCategoryNum = inputData.serverCategoryNum;
            }

            //1.��inputdata�еķ�������Ϣ��ȡ����
            if (startReadServerInfo && index <= serverCategoryNum)
            {
                ss >> inputData.serverInfo[index].name;
                ss >> inputData.serverInfo[index].cpuNum;
                ss >> inputData.serverInfo[index].memNum;
                ss >> inputData.serverInfo[index].diskNum;
            }

            startReadServerInfo = true;

            //2.��ȡ������������
            if (index == (serverCategoryNum + 2))
            {

                ss >> inputData.flavorNum;
                flavorNum = inputData.flavorNum;
            }

            //3.��ȡ�������������Ϣ
            if (index > (serverCategoryNum + 2) && index <= (flavorNum + serverCategoryNum + 2))
            {

                FlavorInfo flavor;

                for (int i = 0; i < 3; ++i)
                {

                    //��ȡ����С
                    int flaNum = 0;
                    if (i == 0)
                    {
                        string flavorName;

                        ss >> flavorName;

                        if (flavorName.length() == 7)
                            flaNum = int(flavorName[6] - '0');
                        else
                            flaNum = 10 * int(flavorName[6] - '0') + int(flavorName[7] - '0');
                        flavor.number = flaNum;
                    }
                    //��ȡ�ù������Ҫ���ڴ��С
                    if (i == 1)
                    {
                        ss >> flavor.needMemNum;
                        flavor.needMemNum /= 1024;
                    }
                    //��ȡ�ù������Ҫ��cpu����
                    else
                    {
                        ss >> flavor.needCpuNum;
                    }

                } //for
                inputData.flavors.push_back(flavor);

            } //if

            //5.��ȡԤ��ʱ��εĿ�ʼ����
            if (index == (flavorNum + serverCategoryNum + 4))
            {

                string date;
                ss >> date;

                int year = 0, month = 0, day = 0;
                for (unsigned int i = 0; i < date.length(); ++i)
                {

                    if (i < 4)
                        year = 10 * year + int(date[i] - '0');

                    if (i > 4 && i < 7)
                        month = 10 * month + int(date[i] - '0');

                    if (i > 7)
                        day = 10 * day + int(date[i] - '0');

                } //for

                //����ʼ��������
                inputData.period.startDate[0] = year;
                inputData.period.startDate[1] = month;
                inputData.period.startDate[2] = day;
            }
            //6.��ȡԤ��ʱ��εĽ�������
            if (index == (flavorNum + serverCategoryNum + 5))
            {

                string date;
                ss >> date;

                int year = 0, month = 0, day = 0;
                for (unsigned int i = 0; i < date.length(); ++i)
                {

                    if (i < 4)
                        year = 10 * year + int(date[i] - '0');

                    if (i > 4 && i < 7)
                        month = 10 * month + int(date[i] - '0');

                    if (i > 7)
                        day = 10 * day + int(date[i] - '0');

                } //for

                //��������������
                inputData.period.endDate[0] = year;
                inputData.period.endDate[1] = month;
                inputData.period.endDate[2] = day;

                //������ȡ����
                break;
            }

        } //if

        //ʹ���������ж�ȡ
        ++index;

    } //while

    //��ȡԤ��ʱ����ڵ�������
    int startMonth = inputData.period.startDate[1];
    int startDay = inputData.period.startDate[2];
    int endMonth = inputData.period.endDate[1];
    int endDay = inputData.period.endDate[2];
    //��ȡԤ��ʱ����
    if (startMonth == endMonth)
        inputData.predictDayCount = endDay - startDay + 1;
    else
        inputData.predictDayCount = getMonthSize(startMonth) - startDay + 1 + endDay;
}

//3.�����ݽ���һ����ȥ��
void dataDenoise()
{

    std::cout << "===============================dataDenoise==========================" << std::endl;

    //1.����ȥ�봦��
    for (int i = 1; i < 19; ++i)
    {

        int month = 0;
        int day = 0;

        int absSize = 10; //����ĳһ���ֵ����ǰ�����ƽ��ֵ����ֵ,������ֵ����Ϊ��һ�����쳣ֵ

        int dayCount = 0;

        int num[366] = {0};

        //��ȡÿһ��ǰ���ȥ��ʱ����
        int dateSize = 7;
        
        //1.ͳ��ѵ�����ݼ��������������ݼ�װ��num������
        int k = 0;
        trainDayCount = 0;
        for (month = 1; month <= trainEndMonth; ++month)
        {

            int monthSize = getMonthSize(month);
            if (month == trainEndMonth)
                for (day = 1; day <= trainEndDay; ++day)
                {

                    num[++k] = flavor[i].count[month][day];
                    ++trainDayCount;
                }

            else
                for (day = 1; day <= monthSize; ++day)
                {
                    num[++k] = flavor[i].count[month][day];
                    ++trainDayCount;
                }
        }

        //2.���û������ڵ���ʽ�������ݽ���һ��ȥ�봦��
        for (month = 1; month <= trainEndMonth; ++month)
        {

            //��ȡÿ���µ�����
            int monthSize;

            if (month == trainEndMonth)
                monthSize = trainEndDay;
            else
                monthSize = getMonthSize(month);

            for (day = 1; day <= monthSize; ++day)
            {

                ++dayCount;
                int leftDayIndex = dayCount - dateSize;
                int rightDayIndex = dayCount + dateSize;
                int dayIndex = 0;

                int n = 0;
                int sum = 0;
                int aveNum = 0;

                //ͳ��ǰ�������
                if (dayCount > dateSize)
                {
                    for (dayIndex = leftDayIndex; dayIndex < dayCount; ++dayIndex)
                    {

                        if (dayIndex > 0)
                        {
                            sum += num[dayIndex];
                            ++n;
                        }
                    }
                }

                //ͳ�ƺ�������
                if (dayCount <= trainDayCount - dateSize)
                {
                    for (dayIndex = dayIndex + 1; dayIndex <= trainDayCount && dayIndex <= rightDayIndex; ++dayIndex)
                    {

                        if (dayIndex > dayCount)
                        {

                            sum += num[dayIndex];
                            ++n;
                        }
                    }
                }

                aveNum = sum / n;

                if (std::abs(num[dayCount] - aveNum) > absSize || num[dayCount] == 0)
                    num[dayCount] = aveNum;
            }

        } //for

        //2.��ȥ�������浽������
        int n = 1;
        for (int month = 1; month <= trainEndMonth; ++month)
        {

            int monthSize;
            if (month == trainEndMonth)
                monthSize = trainEndDay;
            else
                monthSize = getMonthSize(month);

            for (int day = 1; day <= monthSize; ++day)
                flavor[i].count[month][day] = num[n++];
        }

    } //for

    //Ԥ��ʱ��ο�ʼ����
    int startMonth = inputData.period.startDate[1];
    int startDay = inputData.period.startDate[2];

    //3.����ѵ������Ԥ��ʱ���֮���ʱ������
    if(startMonth == trainEndMonth)
        train2Predict_days = startDay - trainEndDay;
    else
        train2Predict_days = getMonthSize(trainEndMonth) - trainEndDay + startDay;

    std::cout << "===============================dataDenoise==========================" << std::endl;
}

//4.Ԥ�����������
void predictVM_ES()
{
    std::cout<<"================predictResult=================="<<std::endl;

    //Ԥ��ʱ�����Ϣ
    // int startMonth = inputData.period.startDate[1];
    // int startDay = inputData.period.startDate[2];

    int timeSize = inputData.predictDayCount;      //Ԥ��ʱ��������

    std::cout<<"==========================train2Predict_days============"<<train2Predict_days<<std::endl;

    for(int i = 0; i < inputData.flavorNum; ++i){
   
        //��ȡ��ҪԤ������������Ϣ
        int flaNum = inputData.flavors[i].number;
        
        int sumDays = train2Predict_days; 

        //1.��ѵ�����ݰ�ʱ��ν��зֶδ���
        std::vector<int> vec_data;

        int n = 0;              //�����������
        int sum = 0;            //�Ը�ʱ�����ڵ����ݽ������

        int sumNearDays = 0;    //��Ԥ��ʱ��ο�ʼ֮ǰһ��ʱ�����ݵĺ�
        int dayCount = 0;

        int monthSize;

        for(int month = 1; month <= trainEndMonth; ++month){
            
            if(month == trainEndMonth)
                monthSize = trainEndDay;
            else
                monthSize = getMonthSize(month);        //��ȡÿ���·ݶ�Ӧ������

            for(int day = 1; day <= monthSize; ++day){

                //���㿪ʼԤ��֮ǰ��һ��ʱ���ڵ����������
                ++dayCount;
                if(trainDayCount - dayCount < timeSize)
                    sumNearDays += flavor[flaNum].count[month][day];

                ++n;
                if(n % timeSize == 0){
                    sum += flavor[flaNum].count[month][day];
                    vec_data.push_back(sum);
                    n = 0;
                    sum = 0;
                }
                else
                    sum += flavor[flaNum].count[month][day];

            }//for

        }//for

        std::cout<<"==============n=============="<<n<<std::endl;
        sumDays += n;
        std::cout<<"==================sumDays================"<<sumDays<<std::endl;

        //2.����һ��ָ��ƽ��
        double s0;

        s0 = (vec_data[0] + vec_data[1] + vec_data[2])/3;   //��ʼ��

        double a = 0.42;        //ָ��ƽ��ϵ��
        vector<double> oneTimeES;

        double s1;
        for(unsigned int i = 0; i < vec_data.size(); ++i){
            s1 = a*vec_data[i] + (1-a)*s0;
            oneTimeES.push_back(s1);
            s0 = s1;
        }

        //3.���ж���ָ��ƽ��
        s0 = (vec_data[0] + vec_data[1] + vec_data[2])/3;   //��ʼ��

        std::vector<double> twoTimeES;

        double s2;
        for(unsigned int i = 0; i < oneTimeES.size(); ++i){
            s2 = a*oneTimeES[i] + (1-a)*s0;
            twoTimeES.push_back(s2);

            s0 = s2;
        }

        double At, Bt;
        int t = twoTimeES.size() - 1;
        double predictNum;

        At = 2*oneTimeES[t] - twoTimeES[t];
        Bt = (a/(1-a))*(oneTimeES[t] - twoTimeES[t]);

        predictNum = At + Bt*(1 + sumDays/timeSize);

        // int len = sumDays/timeSize;
        // if(len >= 1){
        //     sumNearDays = At + Bt*len;
        // }

        //4.��Ԥ������������
        double b = 0.8;
        double res = predictNum*b + sumNearDays*(1-b);

        inputData.flavors[i].predictNum = int(res + 25);
        predictFlavors.push_back(inputData.flavors[i]);
        predictFlavorsCount += inputData.flavors[i].predictNum;

    }//for

    //��ӡԤ����
    std::cout<<predictFlavorsCount<<std::endl;
    for(int i = 0; i < inputData.flavorNum; ++i)
        std::cout<<"flavor"<<predictFlavors[i].number<<" "<<predictFlavors[i].predictNum<<std::endl;

    std::cout<<"================predictResult=================="<<std::endl;
}

vector<double> getEs(vector<double> vec_data, double a, double s){

    std::vector<double> v = vec_data;
    std::vector<double> ret_v;

    //double s0 = (v[0] + v[1] + v[2])/3;
    double s0 = s;
    double s1;

    for(unsigned i = 0; i < v.size(); ++i){
        s1 = a*v[i] + (1 - a)*s0;
        ret_v.push_back(s1);

        s0 = s1;
    }

    return ret_v;
}

void predictVM_test(){

    std::cout<<"================predict_Test_Result=================="<<std::endl;
    
    int timeSize = inputData.predictDayCount;      //Ԥ��ʱ��������

    for(int i = 0; i < inputData.flavorNum; ++i){

        int flaNum = inputData.flavors[i].number;

        std::vector<double> vec_data;
        int monthSize;
        int newTrainDays = trainDayCount;

        //1.������ѵ�����ݼ����浽һ��������
        for(int month = 1; month <= trainEndMonth; ++month){

            if(month == trainEndMonth)
                monthSize = trainEndDay;
            else
                monthSize = getMonthSize(month);

            for(int day = 1; day <= monthSize; ++day){
                vec_data.push_back(flavor[flaNum].count[month][day]);
            }
        }

        //2.���е�һ������ָ��ƽ������
        std::vector<double> oneTimeES, twoTimeES;
        std::vector<double> threeTimeES;

        double At, Bt, Ct;
        int t;
        int dayCount = 0;

        double a = 0.18;
        //double s0 = (vec_data[0] + vec_data[1] + vec_data[2])/3;
        double s0 = vec_data[0];

        oneTimeES = getEs(vec_data, a, s0);
        twoTimeES = getEs(oneTimeES, a, s0);

        //3.�ѵ������Ԥ��ʱ���֮��հ׵�����
        t = twoTimeES.size() - 1;
        At = 2*oneTimeES[t] - twoTimeES[t];
        Bt = (a/(1-a))*(oneTimeES[t] - twoTimeES[t]);

        for(int j = 1; j <= train2Predict_days; ++j){

            double num = At + Bt*j;
            vec_data.push_back(num);
            ++newTrainDays;
        }

        //4.�������Ԥ��ʱ��������һ��ʱ���ڵ�ѵ������
        double nearDaysSum = 0;

        for(unsigned int i = 0; i < vec_data.size(); ++i){

            ++dayCount;
            if((newTrainDays - dayCount) < timeSize){
                nearDaysSum += vec_data[i];
            }
        }

        std::cout<<"dayCount:"<<dayCount<<"  newTrainDays:"<<newTrainDays<<"  vec_data size:"<<vec_data.size()<<endl;

        //5.��϶���ָ��ƽ���������������������Ȩ����Ԥ��ֵ
        double res = 0;
        int predictNum;

        //a = 0.12;
        double b = 0.23;
        
        //��������ָ��
        oneTimeES = getEs(vec_data, a, s0);
        twoTimeES = getEs(oneTimeES, a, s0);

        threeTimeES = getEs(twoTimeES, a, s0);
        t = threeTimeES.size() - 1;
        At = 3*oneTimeES[t] - 3*twoTimeES[t] + threeTimeES[t];
        Bt = (a/(2*pow(1-a, 2)))*((6-5*a)*oneTimeES[t] - 2*(5-4*a)*twoTimeES[t] + (4 - 3*a)*threeTimeES[t]);
        Ct = (pow(a, 2)/(2*pow(1-a, 2)))*(oneTimeES[t] - 2*twoTimeES[t] + threeTimeES[t]);

        for(int k = 1; k <= inputData.predictDayCount; ++k){

            double num = At + Bt*k + Ct*k*k;
            res += num;
        }

        res = res*b + (1 - b)*nearDaysSum;

        predictNum = int(res + 35);


        //6.����Ԥ����ֵ
        inputData.flavors[i].predictNum = predictNum;
        predictFlavors.push_back(inputData.flavors[i]);
        predictFlavorsCount += inputData.flavors[i].predictNum;

        std::cout<<"nearDaysSum:"<<nearDaysSum<<std::endl;
    }//for

    //��ӡԤ����
    std::cout<<"================xxxxxxxxxxxxxxxxxx=================="<<std::endl;
    std::cout<<predictFlavorsCount<<std::endl;
    for(int i = 0; i < inputData.flavorNum; ++i)
        std::cout<<"flavor"<<predictFlavors[i].number<<" "<<predictFlavors[i].predictNum<<std::endl;

    std::cout<<"================predict_Test_Result=================="<<std::endl;
}

//5.����ģ���˻��㷨����������з���
void putFlavors2Server()
{

    std::cout << "==============================================putFlacors2Server==================================" << std::endl;

    vector<FlavorInfo> predictFlavors_vec; //���Ԥ������������
    vector<ServerInfo> needServers;        //�������������������Ҫ�ķ�����
    vector<FlavorInfo> predictFlavors_tmp = predictFlavors;

    std::vector<ServerInfo> serverType; //����������

    double minServer = predictFlavors_vec.size() + 1;

    double T0 = 300;       //���ó�ʼ�¶�
    double minT = 1;       //�����¶�����
    double alpha = 0.9999; //�����˻���½�����

    vector<int> flavorRandNum, serverRandNum;
    int flavorsCount = 0;

    //1.����Ҫ���õ��������Ϣ��ȡ����
    for (unsigned int i = 0; i < predictFlavors.size(); ++i)
    {
        for (int j = 0; j < predictFlavors[i].predictNum; ++j)
        {
            predictFlavors_vec.push_back(predictFlavors[i]);
            ++flavorsCount;
        }
    }
    std::cout << "===================predictFlavorsCount========" << flavorsCount << std::endl;

    //2.������������Ϣ��ȡ����
    for (int i = 1; i <= inputData.serverCategoryNum; ++i)
    {
        serverType.push_back(inputData.serverInfo[i]);
    }

    //3.��ʼһ��������������������������˳���Լ�������������˳��
    for (unsigned int i = 0; i < predictFlavors_vec.size(); ++i)
        flavorRandNum.push_back(i);
    for (int i = 0; i < inputData.serverCategoryNum; ++i)
        serverRandNum.push_back(i);

    best_Result.minNum = 10000;
    //4.����ģ���˻����
    while (T0 > minT)
    {

        std::random_shuffle(flavorRandNum.begin(), flavorRandNum.end());
        std::random_shuffle(serverRandNum.begin(), serverRandNum.end());

        auto new_vec_flavors = predictFlavors_vec;
        auto new_vec_serType = serverType;

        std::swap(new_vec_flavors[flavorRandNum[0]], new_vec_flavors[flavorRandNum[1]]);
        std::swap(new_vec_serType[serverRandNum[0]], new_vec_serType[serverRandNum[1]]);

        vector<ServerInfo> servers;
        ServerInfo firstServer;

        firstServer.name = new_vec_serType[0].name;
        firstServer.cpuNum = new_vec_serType[0].cpuNum;
        firstServer.memNum = new_vec_serType[0].memNum;
        firstServer.freeCpuNum = firstServer.cpuNum;
        firstServer.freeMemNum = firstServer.memNum;

        servers.push_back(firstServer);

        for (auto element : new_vec_flavors)
        {

            unsigned int i;

            for (i = 0; i < servers.size(); ++i)
            {

                if (servers[i].freeCpuNum >= element.needCpuNum && servers[i].freeMemNum >= element.needMemNum)
                {

                    servers[i].freeCpuNum -= element.needCpuNum;
                    servers[i].freeMemNum -= element.needMemNum;
                    servers[i].flavors.push_back(element);

                    break;
                } //if

            } //for

            if (i == servers.size())
            {

                std::random_shuffle(serverRandNum.begin(), serverRandNum.end());
                auto new_vec_serType = serverType;

                std::swap(new_vec_serType[serverRandNum[0]], new_vec_serType[serverRandNum[1]]);

                ServerInfo newServer;
                newServer.name = new_vec_serType[0].name;
                newServer.cpuNum = new_vec_serType[0].cpuNum;
                newServer.memNum = new_vec_serType[0].memNum;
                newServer.freeCpuNum = newServer.cpuNum;
                newServer.freeMemNum = newServer.memNum;

                if (newServer.freeCpuNum >= element.needCpuNum && newServer.freeMemNum >= element.needMemNum)
                {

                    newServer.freeCpuNum -= element.needCpuNum;
                    newServer.freeMemNum -= element.needMemNum;
                    newServer.flavors.push_back(element);

                } //if

                servers.push_back(newServer);

            } //if

        } //for

        double serverNum;
        double sum_grade_cpu = 0;
        double sum_grade_mem = 0;
        double sum_cpu_usage = 0;
        double sum_mem_usage = 0;

        for (auto element : servers)
        {

            double cpuUseRate = element.cpuNum - element.freeCpuNum;
            double memUseRate = element.memNum - element.freeMemNum;

            sum_cpu_usage += cpuUseRate;
            sum_mem_usage += memUseRate;
            sum_grade_cpu += element.cpuNum;
            sum_grade_mem += element.memNum;
        }

        //�÷�Խ��Խ��
        serverNum = 1 - (sum_cpu_usage / sum_grade_cpu + sum_mem_usage / sum_grade_mem) / 2;

        if (serverNum < minServer)
        {

            minServer = serverNum;
            needServers = servers;
            predictFlavors_vec = new_vec_flavors;
        }
        else
        {
            if (std::exp((minServer - serverNum) / T0) > rand() / RAND_MAX)
            {
                if (best_Result.minNum > minServer)
                {
                    best_Result.minNum = minServer;
                    best_Result.best_servers = needServers;
                    best_Result.best_vec_flavors = predictFlavors_vec;
                }
                minServer = serverNum;
                needServers = servers;
                predictFlavors_vec = new_vec_flavors;
            }
        }
        if (best_Result.minNum < minServer)
        {
            needServers = best_Result.best_servers;
        }
        T0 = alpha * T0; //�˻����

    } //while

    std::cout<<"===================================freeMemNum and freeCpuNum========================="<<std::endl;
    //5.����������и����͵�����
    for (auto server : needServers)
    {

        //����������и����������������
        if (server.name == "General")
            ++needNum[0];
        else if (server.name == "Large-Memory")
            ++needNum[1];
        else if (server.name == "High-Performance")
            ++needNum[2];

        std::cout<<server.name<<":";
        std::cout<<"("<<server.freeCpuNum<<", "<<server.freeMemNum<<")"<<std::endl;
        //res_servers.push_back(server);

    }
    std::cout<<"============================================================"<<std::endl;

    //�������һ̨�����
    int getServernum[3] = {0};
    for(auto server : needServers){

        if(server.name == "General"){
            ++getServernum[0];
            if(getServernum[0] < needNum[0])
                res_servers.push_back(server);
            else{
                for(auto flavor : server.flavors){

                    for(unsigned int i = 0; i < predictFlavors.size(); ++i){
                        if(predictFlavors[i].number == flavor.number)
                            --predictFlavors[i].predictNum;
                    }

                    --predictFlavorsCount;
                }
            }
        }
        else if (server.name == "Large-Memory"){
            ++getServernum[1];
            if(getServernum[1] < needNum[1])
                res_servers.push_back(server);
            else{
                for(auto flavor : server.flavors){

                    for(unsigned int i = 0; i < predictFlavors.size(); ++i){
                        if(predictFlavors[i].number == flavor.number)
                            --predictFlavors[i].predictNum;
                    }

                    --predictFlavorsCount;
                }
            }
        }
        else if (server.name == "High-Performance"){
            ++getServernum[2];
            if(getServernum[2] < needNum[2])
                res_servers.push_back(server);
            else{
                for(auto flavor : server.flavors){

                    for(unsigned int i = 0; i < predictFlavors.size(); ++i){
                        if(predictFlavors[i].number == flavor.number)
                            --predictFlavors[i].predictNum;
                    }

                    --predictFlavorsCount;
                }
            } 
        }

    }//for

    for(int j = 0; j < 3; ++j)
        if(needNum[j] > 0)
            --needNum[j];

    for (auto server : res_servers)
    {
        std::cout<<server.name<<":";
        std::cout<<"("<<server.freeCpuNum<<", "<<server.freeMemNum<<")"<<std::endl;

    }
    std::cout<<"===================================freeMemNum and freeCpuNum========================="<<std::endl;


    //6.��ÿ��������ϵ��������������һ���ۼƲ���
    for (unsigned int i = 0; i < res_servers.size(); ++i)
    {

        std::vector<FlavorInfo> fla;
        for (auto flavor : res_servers[i].flavors)
        {

            flavor.putNum = 0;

            int flag = 0;

            if (!fla.empty())
            {
                for (unsigned int j = 0; j < fla.size(); ++j)
                {

                    if (flavor.number == fla[j].number)
                    {

                        ++fla[j].putNum;
                        flag = 1;
                        break;
                    }
                }
            }

            if (flag == 0)
            {
                flavor.putNum = 1;
                fla.push_back(flavor);
            }

        } //for

        res_servers[i].flavors = fla;

    } //for

    //7.��ͳ�ƺ÷�����������������������浽���շ�������
    for (auto server : res_servers)
    {

        if (server.name == "General")
            putServers[0].push_back(server);
        if (server.name == "Large-Memory")
            putServers[1].push_back(server);
        if (server.name == "High-Performance")
            putServers[2].push_back(server);
    }

    //��ӡ������
    // for(int i = 0; i < 3; ++i){
    //     for(auto server : putServers[i]){
    //         std::cout<<server.name<<":"<<std::endl;
    //         for(auto flavor : server.flavors){
    //             std::cout<<"flavor"<<flavor.number<<" "<<flavor.putNum<<" ";
    //         }
    //         std::cout<<std::endl;
    //     } 
    // }

    // //��ӡԤ����
    // std::cout<<"================xxxxxxxxxxxxxxxxxx=================="<<std::endl;
    // std::cout<<predictFlavorsCount<<std::endl;
    // for(int i = 0; i < inputData.flavorNum; ++i)
    //     std::cout<<"flavor"<<predictFlavors[i].number<<" "<<predictFlavors[i].predictNum<<std::endl;

    std::cout << "==============================================putFlacors2Server==================================" << std::endl;
}

//��ӡtrainData��ͳ�ƽ��
void printTrainData()
{

    for (int i = 1; i < 19; ++i)
    {
        std::cout << "flavor" << i << ":" << std::endl;
        for (int month = 1; month < 13; ++month)
        {
            for (int day = 1; day < 32; ++day)
            {
                std::cout << flavor[i].count[month][day] << " ";
            }
            std::cout << std::endl;
        }
    }
}

//��ӡ��inputdata����������Ϣ
void printInputData()
{

    std::cout << "================inputData==================" << std::endl;

    int num = inputData.serverCategoryNum;

    //1.��������Ϣ
    std::cout << num << std::endl;

    for (int i = 1; i <= num; ++i)
    {
        std::cout << inputData.serverInfo[i].name << " " << inputData.serverInfo[i].cpuNum << " " << inputData.serverInfo[i].memNum << " " << inputData.serverInfo[i].diskNum << std::endl;
    }

    //2.���������
    std::cout << inputData.flavorNum << std::endl;

    //3.ÿ������������Ҫ����Դ��Ϣ
    for (int i = 0; i < inputData.flavorNum; ++i)
    {
        std::cout << "flavor" << inputData.flavors[i].number << " " << inputData.flavors[i].needCpuNum << " " << inputData.flavors[i].needMemNum << std::endl;
    }

    //4.��ӡ��Ԥ��ʱ���
    std::cout << inputData.period.startDate[0] << "-" << inputData.period.startDate[1] << "-" << inputData.period.startDate[2] << std::endl;
    std::cout << inputData.period.endDate[0] << "-" << inputData.period.endDate[1] << "-" << inputData.period.endDate[2] << std::endl;

    std::cout << "================inputData==================" << std::endl;
}

//��ȡÿ���·ݶ�Ӧ������
int getMonthSize(int month)
{

    //�ж�ÿ���µ�����
    int monthSize = 0;
    if (month <= 7 && month % 2 == 1)
        monthSize = 31;
    if (month <= 7 && month % 2 == 0)
        monthSize = 30;
    if (month == 2)
        monthSize = 28;
    if (month >= 8 && month % 2 == 0)
        monthSize = 31;
    if (month >= 8 && month % 2 == 1)
        monthSize = 30;

    return monthSize;
}
