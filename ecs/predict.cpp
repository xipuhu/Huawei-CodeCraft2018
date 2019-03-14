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

//trainData中的虚拟机统计信息
struct Flavor
{
    int count[13][32];
    int year;
};

//inputData中的虚拟机信息
struct FlavorInfo
{
    int number;     //该虚拟机编号
    int needCpuNum; //该虚拟机规格需要的cpu核数
    int needMemNum; //该虚拟机规格需要的内存大小

    int predictNum; //该虚拟机预测出来的数量
    int putNum;     //该虚拟机分配的数量
};

//inputData中的服务器信息
struct ServerInfo
{

    string name; //服务器类型名称

    int cpuNum;  //cpu核数
    int memNum;  //内存大小
    int diskNum; //硬盘数量

    int freeCpuNum; //分配时剩余的cpu核数
    int freeMemNum; //分配时剩余的内存大小

    vector<FlavorInfo> flavors; //需要预测的虚拟机信息

    int needNum; //分配过程中需要使用该服务器的数量
};

//inputData中预测时间段信息
struct Period
{
    int startDate[3]; //数组元素分别表示:年,月,日
    int endDate[3];
};

//inputData
struct InputData
{
    int serverCategoryNum; //服务器种类数量

    ServerInfo serverInfo[4]; //三种类型的服务器

    int flavorNum; //需要预测的虚拟机规格数量

    std::vector<FlavorInfo> flavors; //每个虚拟机规格的具体信息

    Period period;       //需要预测的时间段
    int predictDayCount; //预测时间段的总天数
};

//训练数据集的虚拟机规格信息
Flavor flavor[19];
int trainEndMonth; //记录训练数据集的结束日期
int trainEndDay;
int trainDayCount; //统计训练数据集总共有多少天

//计算训练集到预测时间段之间的时间天数
int train2Predict_days;

//input数据信息
InputData inputData;

//对每个虚拟机预测的结果
vector<FlavorInfo> predictFlavors;
int predictFlavorsCount; //预测出来的虚拟机总数量

//分配虚拟机的结果
std::vector<ServerInfo> res_servers;
std::vector<ServerInfo> putServers[3];
int needNum[3];

//模拟退火中需要用到的结构体变量
struct Best_Result
{
    double minNum;
    vector<ServerInfo> best_servers;
    vector<FlavorInfo> best_vec_flavors;
};
Best_Result best_Result;

//你要完成的功能总入口
void predict_server(char *info[MAX_INFO_NUM], char *data[MAX_DATA_NUM], int data_num, char *filename)
{

    //获取训练数据
    getTrainData(data, data_num);
    //打印训练数据
    printTrainData();

    //获取input数据
    getInputData(info);
    //打印出input数据信息
    printInputData();

    //对数据进行一个简单去噪
    dataDenoise();

    //预测虚拟机数量
    //predictVM_ES();
    predictVM_test();

    //将预测出来的虚拟机分配到服务器上
    putFlavors2Server();

    //将预测结果以及分配结果保存到文件
    string result_str = "";

    std::ostringstream ostr; //声明字符串流用于字符串的读取与保存

    //1.将预测结果保存
    ostr << predictFlavorsCount; //将预测的总虚拟机数量保存到result_str中
    ostr << "\n";
    result_str += ostr.str();

    for (int i = 0; i < inputData.flavorNum; ++i)
    {

        ostr.str(""); //清空字符串流

        ostr << "flavor";
        ostr << predictFlavors[i].number;
        ostr << " ";
        ostr << predictFlavors[i].predictNum;
        ostr << "\n";

        result_str += ostr.str();
    }

    //2.将分配结果保存
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
    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(result_file, filename);
    std::cout << "=============================The last result===========================" << std::endl;
}
//1.从文本中获取训练数据
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
                //获取规格大小
                if(n == 1){
                    
                    if(trainDataUnit.length() == 7)
                        flaNum = int(trainDataUnit[6] - '0');    
                    else
                        flaNum = 10*int(trainDataUnit[6] - '0') + int(trainDataUnit[7] - '0');   
                }
                //获取时间
                if(n == 2){
                    month = 10*int(trainDataUnit[5] - '0') + int(trainDataUnit[6] - '0');
                    day = 10*int(trainDataUnit[8] - '0') + int(trainDataUnit[9] - '0');
                }

            }//for
            
            //根据规格大小和所对应的时间进行相应的统计
            if(flaNum <= 18)
                flavor[flaNum].count[month][day] += 1;

            //记录训练数据集的结束日期
            trainEndMonth = month;
            trainEndDay = day;
        }//if
            
    }//for
}

//2.获取input文件中的数据信息
void getInputData(char *info[MAX_INFO_NUM])
{

    int index = 0;
    int flavorNum = 0;

    int serverCategoryNum = 5;        //服务器种类数量
    bool startReadServerInfo = false; //读取服务器信息的开关

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

            //1.将inputdata中的服务器信息读取出来
            if (startReadServerInfo && index <= serverCategoryNum)
            {
                ss >> inputData.serverInfo[index].name;
                ss >> inputData.serverInfo[index].cpuNum;
                ss >> inputData.serverInfo[index].memNum;
                ss >> inputData.serverInfo[index].diskNum;
            }

            startReadServerInfo = true;

            //2.读取虚拟机规格数量
            if (index == (serverCategoryNum + 2))
            {

                ss >> inputData.flavorNum;
                flavorNum = inputData.flavorNum;
            }

            //3.读取虚拟机规格具体信息
            if (index > (serverCategoryNum + 2) && index <= (flavorNum + serverCategoryNum + 2))
            {

                FlavorInfo flavor;

                for (int i = 0; i < 3; ++i)
                {

                    //获取规格大小
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
                    //获取该规格所需要的内存大小
                    if (i == 1)
                    {
                        ss >> flavor.needMemNum;
                        flavor.needMemNum /= 1024;
                    }
                    //获取该规格所需要的cpu核数
                    else
                    {
                        ss >> flavor.needCpuNum;
                    }

                } //for
                inputData.flavors.push_back(flavor);

            } //if

            //5.读取预测时间段的开始日期
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

                //将开始日期载入
                inputData.period.startDate[0] = year;
                inputData.period.startDate[1] = month;
                inputData.period.startDate[2] = day;
            }
            //6.读取预测时间段的结束日期
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

                //将结束日期载入
                inputData.period.endDate[0] = year;
                inputData.period.endDate[1] = month;
                inputData.period.endDate[2] = day;

                //结束读取数据
                break;
            }

        } //if

        //使数据流换行读取
        ++index;

    } //while

    //获取预测时间段内的总天数
    int startMonth = inputData.period.startDate[1];
    int startDay = inputData.period.startDate[2];
    int endMonth = inputData.period.endDate[1];
    int endDay = inputData.period.endDate[2];
    //获取预测时间间隔
    if (startMonth == endMonth)
        inputData.predictDayCount = endDay - startDay + 1;
    else
        inputData.predictDayCount = getMonthSize(startMonth) - startDay + 1 + endDay;
}

//3.对数据进行一个简单去噪
void dataDenoise()
{

    std::cout << "===============================dataDenoise==========================" << std::endl;

    //1.进行去噪处理
    for (int i = 1; i < 19; ++i)
    {

        int month = 0;
        int day = 0;

        int absSize = 10; //设置某一天的值与其前后几天的平均值最大差值,超出该值则认为这一天是异常值

        int dayCount = 0;

        int num[366] = {0};

        //获取每一天前后的去噪时间间隔
        int dateSize = 7;
        
        //1.统计训练数据集的天数并将数据集装进num数组中
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

        //2.采用滑动窗口的形式来对数据进行一个去噪处理
        for (month = 1; month <= trainEndMonth; ++month)
        {

            //获取每个月的天数
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

                //统计前半个窗口
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

                //统计后半个窗口
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

        //2.将去噪结果保存到数据中
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

    //预测时间段开始日期
    int startMonth = inputData.period.startDate[1];
    int startDay = inputData.period.startDate[2];

    //3.计算训练集到预测时间段之间的时间天数
    if(startMonth == trainEndMonth)
        train2Predict_days = startDay - trainEndDay;
    else
        train2Predict_days = getMonthSize(trainEndMonth) - trainEndDay + startDay;

    std::cout << "===============================dataDenoise==========================" << std::endl;
}

//4.预测虚拟机数量
void predictVM_ES()
{
    std::cout<<"================predictResult=================="<<std::endl;

    //预测时间段信息
    // int startMonth = inputData.period.startDate[1];
    // int startDay = inputData.period.startDate[2];

    int timeSize = inputData.predictDayCount;      //预测时间间隔长度

    std::cout<<"==========================train2Predict_days============"<<train2Predict_days<<std::endl;

    for(int i = 0; i < inputData.flavorNum; ++i){
   
        //获取需要预测的虚拟机的信息
        int flaNum = inputData.flavors[i].number;
        
        int sumDays = train2Predict_days; 

        //1.将训练数据按时间段进行分段处理
        std::vector<int> vec_data;

        int n = 0;              //计算天数间隔
        int sum = 0;            //对该时间间隔内的数据进行求和

        int sumNearDays = 0;    //对预测时间段开始之前一段时间数据的和
        int dayCount = 0;

        int monthSize;

        for(int month = 1; month <= trainEndMonth; ++month){
            
            if(month == trainEndMonth)
                monthSize = trainEndDay;
            else
                monthSize = getMonthSize(month);        //获取每个月份对应的天数

            for(int day = 1; day <= monthSize; ++day){

                //计算开始预测之前的一段时间内的虚拟机数量
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

        //2.进行一次指数平滑
        double s0;

        s0 = (vec_data[0] + vec_data[1] + vec_data[2])/3;   //初始化

        double a = 0.42;        //指数平滑系数
        vector<double> oneTimeES;

        double s1;
        for(unsigned int i = 0; i < vec_data.size(); ++i){
            s1 = a*vec_data[i] + (1-a)*s0;
            oneTimeES.push_back(s1);
            s0 = s1;
        }

        //3.进行二次指数平滑
        s0 = (vec_data[0] + vec_data[1] + vec_data[2])/3;   //初始化

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

        //4.将预测结果保存起来
        double b = 0.8;
        double res = predictNum*b + sumNearDays*(1-b);

        inputData.flavors[i].predictNum = int(res + 25);
        predictFlavors.push_back(inputData.flavors[i]);
        predictFlavorsCount += inputData.flavors[i].predictNum;

    }//for

    //打印预测结果
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
    
    int timeSize = inputData.predictDayCount;      //预测时间间隔长度

    for(int i = 0; i < inputData.flavorNum; ++i){

        int flaNum = inputData.flavors[i].number;

        std::vector<double> vec_data;
        int monthSize;
        int newTrainDays = trainDayCount;

        //1.将所有训练数据集保存到一个向量中
        for(int month = 1; month <= trainEndMonth; ++month){

            if(month == trainEndMonth)
                monthSize = trainEndDay;
            else
                monthSize = getMonthSize(month);

            for(int day = 1; day <= monthSize; ++day){
                vec_data.push_back(flavor[flaNum].count[month][day]);
            }
        }

        //2.进行第一个二次指数平滑计算
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

        //3.填补训练集与预测时间段之间空白的数据
        t = twoTimeES.size() - 1;
        At = 2*oneTimeES[t] - twoTimeES[t];
        Bt = (a/(1-a))*(oneTimeES[t] - twoTimeES[t]);

        for(int j = 1; j <= train2Predict_days; ++j){

            double num = At + Bt*j;
            vec_data.push_back(num);
            ++newTrainDays;
        }

        //4.计算距离预测时间段最近的一段时间内的训练数据
        double nearDaysSum = 0;

        for(unsigned int i = 0; i < vec_data.size(); ++i){

            ++dayCount;
            if((newTrainDays - dayCount) < timeSize){
                nearDaysSum += vec_data[i];
            }
        }

        std::cout<<"dayCount:"<<dayCount<<"  newTrainDays:"<<newTrainDays<<"  vec_data size:"<<vec_data.size()<<endl;

        //5.结合二次指数平滑和最近天数的数据来加权计算预测值
        double res = 0;
        int predictNum;

        //a = 0.12;
        double b = 0.23;
        
        //进行三次指数
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


        //6.保存预测结果值
        inputData.flavors[i].predictNum = predictNum;
        predictFlavors.push_back(inputData.flavors[i]);
        predictFlavorsCount += inputData.flavors[i].predictNum;

        std::cout<<"nearDaysSum:"<<nearDaysSum<<std::endl;
    }//for

    //打印预测结果
    std::cout<<"================xxxxxxxxxxxxxxxxxx=================="<<std::endl;
    std::cout<<predictFlavorsCount<<std::endl;
    for(int i = 0; i < inputData.flavorNum; ++i)
        std::cout<<"flavor"<<predictFlavors[i].number<<" "<<predictFlavors[i].predictNum<<std::endl;

    std::cout<<"================predict_Test_Result=================="<<std::endl;
}

//5.采用模拟退火算法对虚拟机进行分配
void putFlavors2Server()
{

    std::cout << "==============================================putFlacors2Server==================================" << std::endl;

    vector<FlavorInfo> predictFlavors_vec; //存放预测出来的虚拟机
    vector<ServerInfo> needServers;        //保存用于配置虚拟机需要的服务器
    vector<FlavorInfo> predictFlavors_tmp = predictFlavors;

    std::vector<ServerInfo> serverType; //服务器种类

    double minServer = predictFlavors_vec.size() + 1;

    double T0 = 300;       //设置初始温度
    double minT = 1;       //设置温度下限
    double alpha = 0.9999; //设置退火的下降速率

    vector<int> flavorRandNum, serverRandNum;
    int flavorsCount = 0;

    //1.将需要配置的虚拟机信息读取进来
    for (unsigned int i = 0; i < predictFlavors.size(); ++i)
    {
        for (int j = 0; j < predictFlavors[i].predictNum; ++j)
        {
            predictFlavors_vec.push_back(predictFlavors[i]);
            ++flavorsCount;
        }
    }
    std::cout << "===================predictFlavorsCount========" << flavorsCount << std::endl;

    //2.将服务器的信息读取进来
    for (int i = 1; i <= inputData.serverCategoryNum; ++i)
    {
        serverType.push_back(inputData.serverInfo[i]);
    }

    //3.初始一个向量用来随机产生虚拟机交换顺序以及服务器交换的顺序
    for (unsigned int i = 0; i < predictFlavors_vec.size(); ++i)
        flavorRandNum.push_back(i);
    for (int i = 0; i < inputData.serverCategoryNum; ++i)
        serverRandNum.push_back(i);

    best_Result.minNum = 10000;
    //4.进行模拟退火操作
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

        //得分越低越好
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
        T0 = alpha * T0; //退火操作

    } //while

    std::cout<<"===================================freeMemNum and freeCpuNum========================="<<std::endl;
    //5.计算分配结果中各类型的数量
    for (auto server : needServers)
    {

        //计算分配结果中各类型物理机的数量
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

    //砍掉最后一台物理机
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


    //6.对每种物理机上的虚拟机数量进行一个累计操作
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

    //7.将统计好分配的虚拟机数量的物理机保存到最终分配结果中
    for (auto server : res_servers)
    {

        if (server.name == "General")
            putServers[0].push_back(server);
        if (server.name == "Large-Memory")
            putServers[1].push_back(server);
        if (server.name == "High-Performance")
            putServers[2].push_back(server);
    }

    //打印分配结果
    // for(int i = 0; i < 3; ++i){
    //     for(auto server : putServers[i]){
    //         std::cout<<server.name<<":"<<std::endl;
    //         for(auto flavor : server.flavors){
    //             std::cout<<"flavor"<<flavor.number<<" "<<flavor.putNum<<" ";
    //         }
    //         std::cout<<std::endl;
    //     } 
    // }

    // //打印预测结果
    // std::cout<<"================xxxxxxxxxxxxxxxxxx=================="<<std::endl;
    // std::cout<<predictFlavorsCount<<std::endl;
    // for(int i = 0; i < inputData.flavorNum; ++i)
    //     std::cout<<"flavor"<<predictFlavors[i].number<<" "<<predictFlavors[i].predictNum<<std::endl;

    std::cout << "==============================================putFlacors2Server==================================" << std::endl;
}

//打印trainData的统计结果
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

//打印出inputdata所有数据信息
void printInputData()
{

    std::cout << "================inputData==================" << std::endl;

    int num = inputData.serverCategoryNum;

    //1.服务器信息
    std::cout << num << std::endl;

    for (int i = 1; i <= num; ++i)
    {
        std::cout << inputData.serverInfo[i].name << " " << inputData.serverInfo[i].cpuNum << " " << inputData.serverInfo[i].memNum << " " << inputData.serverInfo[i].diskNum << std::endl;
    }

    //2.虚拟机数量
    std::cout << inputData.flavorNum << std::endl;

    //3.每个虚拟机规格需要的资源信息
    for (int i = 0; i < inputData.flavorNum; ++i)
    {
        std::cout << "flavor" << inputData.flavors[i].number << " " << inputData.flavors[i].needCpuNum << " " << inputData.flavors[i].needMemNum << std::endl;
    }

    //4.打印出预测时间段
    std::cout << inputData.period.startDate[0] << "-" << inputData.period.startDate[1] << "-" << inputData.period.startDate[2] << std::endl;
    std::cout << inputData.period.endDate[0] << "-" << inputData.period.endDate[1] << "-" << inputData.period.endDate[2] << std::endl;

    std::cout << "================inputData==================" << std::endl;
}

//获取每个月份对应的天数
int getMonthSize(int month)
{

    //判断每个月的天数
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
