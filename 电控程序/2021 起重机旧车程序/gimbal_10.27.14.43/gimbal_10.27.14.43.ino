/*
  10.4 20.44 夹取三个箱子动作成功（不带robomodule）
  10.5 11.10 夹取没问题 放置顺序有问题
  10.5 11.34 夹取动作时气缸延迟时间过长
  10.5 17.00 优化气缸延迟时间过长问题
  10.9.22.12 优化夹取过程
  接受视觉信号
  大写是红色 小写蓝色
  t:triangle  q:square  r:circle

  10.10.11.27 完善夹取阶段性的程序
  2021.10.18 基本完成全程
*/
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#define blue_circle 1
#define blue_triangle 2
#define blue_square 3
#define red_circle 4
#define red_triangle 5
#define red_square 6
#define move_position_delay 5000

#define circle_put_in_delay 8000
#define triangle_put_in_delay 5500
#define square_put_in_delay 4200
//3800
#define circle_put_out_delay 4500
#define triangle_put_out_delay 6000
#define square_put_out_delay 7700

#define square_position 23200   //定义robomodule圆放置的位置
#define triangle_position 36100 //定义robomodule三角形放置的位置
#define circle_position 50300   //定义robomodule正方形放置的位置
const int SPI_CS_PIN = 53;
MCP_CAN CAN(SPI_CS_PIN);
char mreset[8];
char msetp[8];
char mpmodea[8];
int temp_pwm = 5000;
long run_positiona = 10000; //QC
int box_position[7];
int box_info = 0;
int box_order = 0;
int pick_stop_num = 0;
int get_info_from_pi = 0;
void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial3.begin(115200); // PI
    Serial2.begin(38400);  //连接蓝牙与底盘通讯
    box_position[blue_circle] = 0;
    box_position[blue_triangle] = 0;
    box_position[blue_square] = 0;
    box_position[red_circle] = 0;
    box_position[red_triangle] = 0;
    box_position[red_square] = 0;
    CAN.begin(CAN_1000KBPS, MCP_8MHz);
    mreset[0] = 0x55;
    mreset[1] = 0x55;
    mreset[2] = 0x55;
    mreset[3] = 0x55;
    mreset[4] = 0x55;
    mreset[5] = 0x55;
    mreset[6] = 0x55;
    mreset[7] = 0x55;
    delay(500);
    CAN.sendMsgBuf(0x000, 0, 8, mreset);
    delay(500);
    msetp[0] = 0x04;
    msetp[1] = 0x55;
    msetp[2] = 0x55;
    msetp[3] = 0x55;
    msetp[4] = 0x55;
    msetp[5] = 0x55;
    msetp[6] = 0x55;
    msetp[7] = 0x55;
    CAN.sendMsgBuf(0x011, 0, 8, msetp);
    delay(500);
    pinMode(22, OUTPUT); //旋转
    pinMode(24, OUTPUT); //夹取
    pinMode(26, OUTPUT); //起升
    digitalWrite(22, HIGH);
    digitalWrite(24, LOW);
    digitalWrite(26, HIGH); //初始化气缸
    pick_from_ground_position();
    cylinder_pick_put();        //张开 松
    cylinder_spin_lengthways(); //旋转平行
    /*
    mreset[0] = 0x55;
    mreset[1] = 0x55;
    mreset[2] = 0x55;
    mreset[3] = 0x55;
    mreset[4] = 0x55;
    mreset[5] = 0x55;
    mreset[6] = 0x55;
    mreset[7] = 0x55;
    CAN.sendMsgBuf(0x000, 0, 8, mreset);
    delay(500);
    msetp[0] = 0x04;
    msetp[1] = 0x55;
    msetp[2] = 0x55;
    msetp[3] = 0x55;
    msetp[4] = 0x55;
    msetp[5] = 0x55;
    msetp[6] = 0x55;
    msetp[7] = 0x55;
    CAN.sendMsgBuf(0x011, 0, 8, msetp);
    delay(500);
    */
}

void loop()
{
   Strat();
}
void Strat()
{

    Serial.println("第一轮第一个");
    pick_and_detect_first_round_box(); //第一轮第一个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第一轮第二个");
    pick_and_detect_first_round_box(); //第一轮第二个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第一轮第三个");
    pick_and_detect_first_round_box(); //第一轮第三个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第一轮第四个");
    pick_and_detect_first_round_box(); //第一轮第四个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第一轮第五个");
    pick_and_detect_first_round_box(); //第一轮第五个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第一轮第六个");
    pick_and_detect_first_round_box(); //第一轮第六个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第一轮放置第一个");
    box_info = blue_triangle; //第一轮三角形放置位置
    put_out_first_triangle();

    Serial.println("第一轮放置第二个");
    box_info = blue_circle;
    lay_position();
    pick_and_put_out_delay();
    Serial.print('G');
    Serial2.print('G');               //车移动到第二个放置位置
    put_out_first_circle_or_square(); //第一轮圆形放置位置

    Serial.println("第一轮放置第三个");
    box_info = blue_square;
    lay_position();
    pick_and_put_out_delay();
    Serial.print('G');
    Serial2.print('G');               //车移动到第三个放置位置
    put_out_first_circle_or_square(); //第一轮正方形放置位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮第一个");
    pick_from_ground_position();
    pick_and_put_second_round_box(); //第二轮第一个暂停位置

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮第二个");

    pick_and_put_second_round_box(); //第二个

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮第三个");
    pick_and_put_second_round_box(); //第三个

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮第四个");

    pick_and_put_second_round_box(); //第四个

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮第五个");

    pick_and_put_second_round_box(); //第五个

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮第六个");

    pick_and_put_second_round_box(); //第六个

    Serial.print('G');
    Serial2.print('G');

    Serial.println("第二轮放置第一个");
    box_info = red_triangle; //第一轮三角形放置位置
    put_out_second_triangle();
    Serial.print('G');
    Serial2.print('G');
    delay(500);

    Serial.println("第二轮放置第二个");
    box_info = red_circle;
    put_out_second_circle_or_square(); //第一轮圆形放置位置
    Serial.print('G');
    Serial2.print('G');
    delay(300);

    Serial.println("第二轮放置第三个");
    box_info = red_square;
    put_out_second_circle_or_square(); //第一轮正方形放置位置
    Serial.print('G');
    Serial2.print('G');
    while (1)
        ;
    /*run_positiona = 0;
    POSITION();
    POSITION();
    while (1)
    ;*/
}
void pick_and_put_second_round_box()
{
    char SerialRead = '\0';
    int recieved = 0;
    cylinder_pick_put();        //张开 松
    cylinder_spin_lengthways(); //旋转平行
    box_info = 0;
    while (Serial2.read() >= 0)
    {
    }
    do
    {
        Serial.println("等待底盘信息");
        if (Serial2.available())
        { //接收底盘已停止信息
            SerialRead = Serial2.read();
            if (SerialRead == 'S')
            {
                recieved = 1;
                Serial.println("已收到底盘信息");
                Serial.print("SerialRead:");
                Serial.println(SerialRead);
            }
        }
    } while (recieved == 0);
    pick_stop_num++;
    if (box_position[pick_stop_num] == red_circle || box_position[pick_stop_num] == red_triangle || box_position[pick_stop_num] == red_square)
    {
        box_info = box_position[pick_stop_num];
        pick_and_put_in();
        if (pick_stop_num != 6)
        {
            pick_from_ground_position();
            pick_and_put_in_delay();
        }
    }
}
void put_out_second_triangle()
{
    char SerialRead = '\0';

    lay_position();
    while (Serial2.read() >= 0)
    {
    }
    do
    {
        if (Serial2.available())
        { //接收底盘已停止信息
            SerialRead = Serial2.read();
        }
    } while (SerialRead != 'S');
    pick_from_car_pick();
    stacking_position();
    pick_and_put_out_delay();
    cylinder_spin_lengthways(); //旋转平行
    delay(1000);
    cylinder_lifting_down(); //下降
    delay(300);
    cylinder_pick_put();   //放置
    cylinder_lifting_up(); //起升
}
void put_out_first_triangle()
{
    char SerialRead = '\0';

    lay_position();
    while (Serial2.read() >= 0)
    {
    }
    do
    {
        if (Serial2.available())
        { //接收底盘已停止信息
            SerialRead = Serial2.read();
        }
    } while (SerialRead != 'S');
    pick_from_car_pick();
    stacking_position();
    pick_and_put_out_delay();
    pick_from_car_put();
}
void put_out_second_circle_or_square()
{
    char SerialRead = '\0';
    while (Serial2.read() >= 0)
    {
    }
    do
    {
        if (Serial2.available())
        { //接收底盘已停止信息
            SerialRead = Serial2.read();
        }
    } while (SerialRead != 'S');
    lay_position();
    pick_and_put_out_delay();
    pick_from_car_pick();
    stacking_position();
    pick_and_put_out_delay();
    cylinder_spin_lengthways(); //旋转平行
    delay(1000);
    cylinder_lifting_down(); //下降
    delay(500);
    cylinder_pick_put();   //放置
    cylinder_lifting_up(); //起升
}
void put_out_first_circle_or_square()
{
    char SerialRead = '\0';
    while (Serial2.read() >= 0)
    {
    }
    do
    {
        if (Serial2.available())
        { //接收底盘已停止信息
            SerialRead = Serial2.read();
        }
    } while (SerialRead != 'S');
    pick_from_car_pick();
    stacking_position();
    pick_and_put_out_delay();
    pick_from_car_put();
}
void pick_and_detect_first_round_box()
{
    char SerialRead = '\0';
    int recieved = 0;
    cylinder_pick_put();        //张开 松
    cylinder_spin_lengthways(); //旋转平行
    while (Serial2.read() >= 0)
    {
    }
    pick_from_ground_position();
    do
    {
        Serial.println("等待底盘信息");
        if (Serial2.available())
        { //接收底盘已停止信息
            SerialRead = Serial2.read();
            if (SerialRead == 'S')
            {
                recieved = 1;
                Serial.println("已收到底盘信息");
                Serial.print("SerialRead:");
                Serial.println(SerialRead);
            }
        }
    } while (recieved == 0);

    /*while (Serial2.read() != 'S')
    {
    Serial.println("等待底盘信息");
    }*/
    box_info = 0;
    get_info_from_pi = 0;
    while (Serial3.read() >= 0)
    {
    }  //清空串口缓存
    do //接收树莓派的信息
    {
        box_info = 0;
        recieve_message_from_pi();
        Serial.println("等待pi信息");
        Serial.print("box_info");
        Serial.println(box_info);
    } while (get_info_from_pi != 1);
    Serial.println("已经接收到pi的信息");
    box_order++;
    box_position[box_order] = box_info;
    Serial.print("box_order：");
    Serial.println(box_order);
    Serial.print("box_position[box_order]：");
    Serial.println(box_position[box_order]);
    if (box_info == blue_circle || box_info == blue_triangle || box_info == blue_square)
    {
        pick_and_put_in();
        if (box_order != 6)
        {
            pick_from_ground_position();
            pick_and_put_in_delay();
        }
    }
}
void recieve_message_from_pi()
{
    char readSerial;

    if (Serial3.available())
    {
        readSerial = Serial3.read();
        switch (readSerial)
        {
        case 't':
            box_info = blue_triangle;
            Serial.print("已检测到蓝色三角");
            get_info_from_pi = 1;
            break;
        case 'T':
            box_info = red_triangle;
            Serial.print("已检测到红色三角");
            get_info_from_pi = 1;
            break;
        case 'r':
            box_info = blue_circle;
            Serial.print("已检测到蓝色圆");
            get_info_from_pi = 1;
            break;
        case 'R':
            box_info = red_circle;
            Serial.print("已检测到红色圆");
            get_info_from_pi = 1;
            break;
        case 'q':
            box_info = blue_square;
            Serial.print("已检测到蓝色正方形");
            get_info_from_pi = 1;
            break;
        case 'Q':
            box_info = red_square;
            Serial.print("已检测到红色正方形");
            get_info_from_pi = 1;
            break;
        }
    }
}

void test()
{
    pick_and_put_in();
    pick_and_put_in();
    pick_and_put_in();

    pick_and_put_out();
    delay(1000);
    pick_and_put_out();
    delay(1000);
    pick_and_put_out();
    delay(1000);
    pick_from_ground_position();
    while (1)
        ;
}
void pick_and_put_in()
{
    pick_form_ground_pick();
    lay_position(); //robomodule运动到对应箱子放置位置
    //if (box_info == blue_circle || box_info == blue_triangle || box_info == blue_square)
    pick_and_put_in_delay();
    pick_form_ground_put();
}
void pick_and_put_in_delay()
{
    switch (box_info)
    {
    case blue_circle:
        delay(circle_put_in_delay);
        break;
    case blue_triangle:
        delay(triangle_put_in_delay);
        break;
    case blue_square:
        delay(square_put_in_delay);
        break;
    case red_circle:
        delay(circle_put_in_delay);
        break;
    case red_triangle:
        delay(triangle_put_in_delay);
        break;
    case red_square:
        delay(square_put_in_delay);
        break;
    }
}
void pick_and_put_out()
{
    lay_position(); //robomodule运动到对应箱子放置位置
    delay(move_position_delay);
    pick_from_car_pick();
    stacking_position();
    pick_and_put_out_delay();
    pick_from_car_put();
}
void pick_and_put_out_delay()
{
    switch (box_info)
    {
    case blue_circle:
        delay(circle_put_out_delay);
        break;
    case blue_triangle:
        delay(triangle_put_out_delay);
        break;
    case blue_square:
        delay(square_put_out_delay);
        break;
    case red_circle:
        delay(circle_put_out_delay);
        break;
    case red_triangle:
        delay(triangle_put_out_delay);
        break;
    case red_square:
        delay(square_put_out_delay);
        break;
    }
}
void POSITION()
{
    mpmodea[0] = (unsigned char)((temp_pwm >> 8) & 0xff);
    mpmodea[1] = (unsigned char)((temp_pwm)&0xff);
    mpmodea[2] = 0x55;
    mpmodea[3] = 0x55;
    mpmodea[4] = (unsigned char)((run_positiona >> 24) & 0xff);
    mpmodea[5] = (unsigned char)((run_positiona >> 16) & 0xff);
    mpmodea[6] = (unsigned char)((run_positiona >> 8) & 0xff);
    mpmodea[7] = (unsigned char)(run_positiona & 0xff);
    CAN.sendMsgBuf(0x015, 0, 8, mpmodea);
}
void pick_form_ground_pick()
{
    cylinder_spin_lengthways(); //旋转平行
    delay(500);
    cylinder_lifting_down(); //下降
    cylinder_pick_put();     //张开 松
    delay(1500);
    cylinder_pick_pick();  //夹取
    cylinder_lifting_up(); //起升
    delay(500);
    cylinder_spin_crosswise(); //旋转垂直
}
void pick_form_ground_put()
{
    cylinder_spin_crosswise(); //旋转垂直
    delay(500);
    cylinder_lifting_down(); //下降
    delay(1000);
    cylinder_pick_put();   //放置
    cylinder_lifting_up(); //起升
    delay(500);
}
void pick_from_car_pick()
{
    cylinder_spin_crosswise(); //旋转垂直
    delay(1000);
    cylinder_lifting_down(); //下降
    delay(1500);
    cylinder_pick_pick();  //夹取
    cylinder_lifting_up(); //起升
    delay(1000);
    cylinder_spin_lengthways(); //旋转平行
}
void pick_from_car_put()
{
    cylinder_spin_lengthways(); //旋转平行
    delay(1000);
    cylinder_lifting_down(); //下降
    delay(1500);
    cylinder_pick_put();   //放置
    cylinder_lifting_up(); //起升
    delay(1200);           //delay(1000);
}
void cylinder_spin_crosswise()
{ //旋转至与丝杠垂直
    digitalWrite(22, HIGH);
}
void cylinder_spin_lengthways()
{ //旋转至与丝杠平行
    digitalWrite(22, LOW);
}
void cylinder_pick_put()
{
    digitalWrite(24, LOW); //放置  开
}
void cylinder_pick_pick()
{
    digitalWrite(24, HIGH); //夹取
}
void cylinder_lifting_down()
{
    digitalWrite(26, LOW); //下降
}
void cylinder_lifting_up()
{ //起升
    digitalWrite(26, HIGH);
}
void lay_position()
{
    switch (box_info)
    {
    case blue_circle:
        run_positiona = circle_position;
        POSITION();
        POSITION();
        Serial.println("26000QC");
        break;
    case blue_triangle:
        run_positiona = triangle_position;
        POSITION();
        POSITION();
        Serial.println("40000QC");
        break;
    case blue_square:
        run_positiona = square_position;
        POSITION();
        POSITION();
        Serial.println("56000QC");
        break;
    case red_circle:
        run_positiona = circle_position;
        POSITION();
        POSITION();
        break;
    case red_triangle:
        run_positiona = triangle_position;
        POSITION();
        POSITION();
        break;
    case red_square:
        run_positiona = square_position;
        POSITION();
        POSITION();
        break;
    }
}
void stacking_position()
{
    run_positiona = 76200;
    POSITION();
    POSITION();
}
void pick_from_ground_position()
{
    run_positiona = -4000;
    POSITION();
    POSITION();
    Serial.println("0QC");
}
void robomodule_test()
{
    for (box_info = 1; box_info < 3; box_info++)
    {
        Serial.println(box_info);
        pick_from_ground_position();
        delay(6000);
        lay_position();
        delay(6000);
    }
    for (box_info = 1; box_info < 3; box_info++)
    {
        lay_position();
        delay(6000);
        stacking_position();
        delay(6000);
    }
}
void Serial_cylinder_test()
{
    int readSerial;
    if (Serial.available())
    {
        readSerial = Serial.read();
        switch (readSerial)
        {
        case '1':
            cylinder_spin_crosswise();
            Serial.println("旋转至与丝杠垂直");
            break;
        case '2':
            cylinder_spin_lengthways();
            Serial.println("旋转至与丝杠平行");
            break;
        case '3':
            cylinder_pick_put();
            Serial.println("张开");
            break;
        case '4':
            cylinder_pick_pick();
            Serial.println("夹取");
            break;
        case '5':
            cylinder_lifting_down();
            Serial.println("下降");
            break;
        case '6':
            cylinder_lifting_up();
            Serial.println("起升");
            break;
        }
    }
}
