/*
    10.5 慢速
           车头
          7    6
     8              4
     9              5
          2   3

   电调ID
           车头
           4    1
           3     2
  2021.10.7.0.32    完成底盘全场巡线分段逻辑
  2021.10.9.14.00   完善底盘全场巡线分段逻辑
  2021.10.10.14.25  完善底盘与云台通讯
  2021.10.11.14     完善1  2  3 阶段
  2021.10.12.11.45  完成一圈巡线
  2021.10.12.17.16  完成两圈巡线
  2021.10.15.3.01   完善两圈巡线3
  2021.10.16.22.03  问题记录 放置三个箱子车身位置倾斜
  2021.10.28.0.12   再次完成全流程
*/
#include <mcp_can.h>
#include <SPI.h>

//#define PRINT
#define blue_circle 1
#define blue_triangle 2
#define blue_square 3
#define red_circle 4
#define red_triangle 5
#define red_square 6

int move_forward_speed = 0;
int move_forward_turn_left_speed = 0;
int move_forward_turn_right_speed = 0;

int move_right_speed = 0;
int move_right_turn_left_speed = 0;
int move_right_turn_right_speed = 0;

int move_back_speed = 1500;
int move_back_turn_speed = 500;

int move_left_speed = 1000;
int move_left_turn_left_speed = 300;
int move_left_turn_right_speed = 300;

int back_line = 0;
int right_line = 0;
int left_line = 0;
int forward_line = 0;
int back_line_last_state = 0;
int right_line_last_state = 0;
int left_line_last_state = 0;
int forward_line_last_state = 0;
int forward = 0, back = 0, left = 0, right = 1;
int box_position[6];
int box_order = 0;
const int SPI_CS_PIN = 53; //设置53号引脚为cs片选
int stage = 1;
int D2state = 0;
int D3state = 0;
int D4state = 0;
int D5state = 0;
int D8state = 0;
int D9state = 0;
int flag = 0;
MCP_CAN CAN(SPI_CS_PIN);
void setup()
{
  box_position[blue_circle] = blue_circle;
  box_position[blue_triangle] = blue_triangle;
  box_position[blue_square] = blue_square;
  box_position[red_circle] = red_circle;
  box_position[red_triangle] = red_triangle;
  box_position[red_square] = red_square;
  pinMode(2, INPUT); //后边左光电
  pinMode(3, INPUT); //后边右光电
  pinMode(4, INPUT); //右边左光电
  pinMode(5, INPUT); //右边右光电
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  CAN.begin(CAN_1000KBPS, MCP_8MHz);
  Serial.begin(115200);
  Serial2.begin(38400); //连接蓝牙与云台通讯
  //#ifdef PRINT
  //#endif
}

void loop()
{
  //send_move_data(1000, -89, -3);
  Start();
}

/*back = 0;
  right = 0;
  left = 1;
  forward = 0;
  patrol();*/
//STOP();
//black_line_test();
//choose_direction();
//recieve_message();
// move_stop();
// 平移左走走直send_move_data(700, -88, -11);
//平移左走走直send_move_data(1000, -88, -11);
//send_move_data(1000, -88, -11);

void Start()
{
  stage_1();
  stage_2();
  stage_3_or_10();
  stage_4_or_11();
  stage_5_or_12();
  stage_6_or_13();
  stage_7_or_14();
  stage_8();
  stage_9();
  stage_3_or_10(); //
  stage_4_or_11();
  stage_5_or_12();
  stage_6_or_13();
  stage_7_or_14();
  send_move_data(1000, -90, 0);
  delay(1000);
  move_forward();
  delay(2000);
  send_move_data(0, 0, 0);
  while (1)
    ;
}
//////////////////////////////巡线子函数/////////////////////////////////
void stage_1()
{
  Serial.println("进入阶段1");
  forward = 0;
  back = 0;
  left = 0;
  right = 1;
  while (stage == 1 && right_line < 2)
  {
    patrol();
    if (digitalRead(4) == 1 && digitalRead(5) == 1)
    {
      delay(20);
      if (digitalRead(4) == 1 && digitalRead(5) == 1 && right_line == 1 && right_line_last_state == 0)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
      }
    }
    right_black_line_test();
    Serial.print("右黑线检测次数");
    Serial.print(right_line);
  }
  stage = 2;
}
void stage_2()
{
  Serial.println("进入阶段2");
  char readSrial = '\0';
  int recieved = 0;
  forward = 1;
  back = 0;
  left = 0;
  right = 0;
  send_move_data(500, 0, 0);
  while (stage == 2 && back_line < 1)
  {
    patrol();
    D2state = digitalRead(2);
    D3state = digitalRead(3);
    Serial.print("if前D2state");
    Serial.println(D2state);
    Serial.print("if前D3state");
    Serial.println(D3state);
    if (D3state && D2state)
    {
      delay(20);
      D2state = digitalRead(2);
      D3state = digitalRead(3);
      Serial.print("if后D2state");
      Serial.println(D2state);
      Serial.print("if后D3state");
      Serial.println(D3state);
      if (D2state && D3state == 1 && back_line == 0 && back_line_last_state == 0)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
      }
    }
    back_black_line_test();
    Serial.print("后黑线检测次数");
    Serial.println(back_line);
  }
  stage = 3;
  Serial2.print('S');
  Serial.println('S');
  while (Serial2.read() >= 0)
  {
  }
  do
  {
    Serial.print("等待接收信号");
    if (Serial2.available())
    {
      readSrial = Serial2.read();
      Serial.println("接收到信号");
      Serial.println(readSrial);
      if (readSrial == 'G')
        recieved = 1;
    }
  } while (recieved != 1);
}
void stage_3_or_10() //夹取阶段
{
  Serial.println("进入阶段3or10");
  forward = 0;
  back = 0;
  left = 1;
  right = 0;
  left_line = 0;
  int recieved = 0;
  char readSrial = '\0';
  left_line = 0;
  send_move_data(1000, -70, -20);
  delay(80);
  while (stage == 3 && left_line < 5)
  {
    //send_move_data(1000, -85, -11);
    patrol();
    D8state = digitalRead(8);
    D9state = digitalRead(9);
    if (D8state && D9state)
    {
      delay(20);
      D8state = digitalRead(8);
      D9state = digitalRead(9);
      if (D8state && D9state && left_line_last_state == 0)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        Serial.println("left_line:");
        Serial.println(left_line);
        delay(3000);
        Serial2.print('S');
        while (Serial2.read() >= 0)
        {
          Serial.print("清空缓存");
        }
        do
        {
          Serial.print("等待接收信号");
          if (Serial2.available())
          {
            readSrial = Serial2.read();
            Serial.print("readSrial:");
            Serial.println(readSrial);
            if (readSrial == 'G')
              recieved = 1;
          }
        } while (recieved != 1);
        readSrial = '\0';
        recieved = 0;
        //左巡线启动三参数 send_move_data(1000, -75, -11);
        //send_move_data(1000, -72, -17);
        send_move_data(1000, -70, -11);
      }
    }
    if (D8state && D9state)
    {
      if (left_line_last_state == 0)
      {
        left_line_last_state = 1;
        left_line++;
      }
    }
    else
    {
      left_line_last_state = 0;
    }
    delay(80);
  }
  stage = 4;
  send_move_data(1000, -185, 0);
}
void stage_4_or_11()
{
  Serial.println("进入阶段4or11");
  forward = 0;
  back = 1;
  left = 0;
  right = 0;
  while (stage == 4)
  {
    patrol();
    D2state = digitalRead(2);
    D3state = digitalRead(3);
    if (D2state && D3state)
    {
      delay(20);
      D2state = digitalRead(2);
      D3state = digitalRead(3);
      if (D2state && D3state)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
        stage = 5;
      }
    }
  }
}
void stage_5_or_12()
{
  Serial.println("进入阶段5or12");
  forward = 0;
  back = 0;
  left = 0;
  right = 1;
  while (stage == 5)
  {
    patrol();
    if (digitalRead(4) == 1 && digitalRead(5) == 1)
    {
      delay(40);
      if (digitalRead(4) == 1 && digitalRead(5) == 1)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
        stage = 6;
      }
    }
  }
}
void stage_6_or_13() ///放置第一个箱子
{
  Serial.println("进入阶段6or13");
  char readSrial = '\0';
  int recieved = 0;
  forward = 0;
  back = 1;
  left = 0;
  right = 0;
  forward_line = 0;
  int D6state = 0;
  int D7state = 0;
  while (stage == 6 && forward_line < 2)
  {
    patrol();
    D6state = digitalRead(6);
    D7state = digitalRead(7);
    if (D6state && D7state)
    {
      delay(20);
      Serial.print("IF前digitalRead(6)");
      Serial.println(D6state);
      Serial.print("digitalRead(7)");
      Serial.println(D7state);
      Serial.print("forward_line");
      Serial.println(forward_line);
      D6state = digitalRead(6);
      D7state = digitalRead(7);
      Serial.print("IF后digitalRead(6)");
      Serial.println(D6state);
      Serial.print("digitalRead(7)");
      Serial.println(D7state);
      Serial.print("forward_line_last_state");
      Serial.println(forward_line_last_state);
      if (D6state && D7state && forward_line == 1 && forward_line_last_state == 0)
      {

        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
      }
    }
    forward_black_line_test();
    Serial.print("forward_line");
    Serial.println(forward_line);
  }
  stage = 7;
  while (Serial2.read() >= 0)
  {
  }
  Serial2.print('S');
  Serial2.print('S');
  do
  {
    Serial.print("等待接收信号");
    if (Serial2.available())
    {
      readSrial = Serial2.read();
      Serial.println("接收到信号");
      Serial.println(readSrial);
      if (readSrial == 'G')
        recieved = 1;
    }
  } while (recieved != 1);
}
void stage_7_or_14() //放置箱子
{
  move_left();
  Serial.println("进入阶段7or14");
  forward = 0;
  back = 0;
  left = 1;
  right = 0;
  left_line = 0;
  char readSrial = '\0';
  int recieved = 0;
  int ini_speed = 0;
  left_line = 0;
  while (stage == 7 && left_line < 2)
  {
    Serial.println("进入阶段7的while");
    patrol();
    D8state = digitalRead(8);
    D9state = digitalRead(9);
    if (D8state == 1 && D9state == 1)
    {
      delay(20);
      D8state = digitalRead(8);
      D9state = digitalRead(9);
      if (D8state == 1 && D9state == 1 && left_line_last_state == 0)
      {
        Serial.print("digitalRead(8)");
        Serial.println(D8state);
        Serial.print("digitalRead(9)");
        Serial.println(D9state);
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
        Serial2.print('S');
        while (Serial2.read() >= 0)
        {
          //Serial.println("清空缓存");
        }
        do
        {
          Serial.println("等待云台信息");
          if (Serial2.available())
          {
            readSrial = Serial2.read();
            Serial.print("readSrial:");
            Serial.println(readSrial);
            if (readSrial == 'G')
              recieved = 1;
            Serial.println("已经接到云台信息");
            //send_move_data(700, -90, -10);
          }
        } while (recieved != 1); //等待云台发送运动指令
        readSrial = '\0';
        recieved = 0;

        //改前send_move_data(700, -85, -11);
        //send_move_data(700, -100, -20);
        /*if (ini_speed == 0)
          {
          send_move_data(700, -90, -10);
          ini_speed = 1;
          }*/
        send_move_data(1000, -85, -11);
      }
    }
    if (D8state && D9state)
    {
      if (left_line_last_state == 0)
      {
        left_line_last_state = 1;
        left_line++;
      }
    }
    else
    {
      left_line_last_state = 0;
    }
    Serial.print("left_line_last_state");
    Serial.println(left_line_last_state);
    delay(100); //
  }
  stage = 8;
}
void stage_8()
{
  Serial.println("进入阶段8");
  forward = 0;
  back = 0;
  left = 0;
  right = 1;
  right_line = 0;
  send_move_data(1000, 90, 200);
  delay(50);
  while (stage == 8 && right_line < 1)
  {
    Serial.println("进入阶段8的while");
    patrol();
    if (digitalRead(4) == 1 && digitalRead(5) == 1)
    {
      delay(20);
      if (digitalRead(4) == 1 && digitalRead(5) == 1 && right_line == 0 && right_line_last_state == 0)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
      }
    }
    right_black_line_test();
    Serial.print("右黑线检测次数");
    Serial.print(right_line);
  }
  stage = 9;
}
void stage_9()
{
  send_move_data(1000, -10, -10 );
  delay(50);
  Serial.println("进入阶段9");
  forward = 1;
  back = 0;
  left = 0;
  right = 0;
  int recieved = 0;
  char readSrial = '\0';
  back_line = 0;
  back_line_last_state = 0;
  flag = 0;
  while (stage == 9 && back_line < 2)
  {
    patrol();
    if (digitalRead(7) && digitalRead(6))
    {
      delay(20);
      if (digitalRead(7) && digitalRead(6))
        flag = 1;
    }
    D2state = digitalRead(2);
    D3state = digitalRead(3);
    if (D2state && D3state)
    {
      delay(20);
      D2state = digitalRead(2);
      D3state = digitalRead(3);
      Serial.print("if后D2state");
      Serial.println(D2state);
      Serial.print("if后D3state");
      Serial.println(D3state);
      if (D2state && D3state && back_line == 1 && back_line_last_state == 0)
      {
        send_move_data(0, 0, 0);
        Serial.println("stop");
        delay(300);
        Serial2.print('S');
      }
    }
    back_black_line_test();
    Serial.print("后黑线检测次数");
    Serial.print(back_line);
  }
  stage = 3;
  while (Serial2.read() >= 0)
  {
  }
  do
  {
    Serial.print("等待接收信号");
    if (Serial2.available())
    {
      readSrial = Serial2.read();
      Serial.println("接收到信号");
      Serial.println(readSrial);
      if (readSrial == 'G')
        recieved = 1;
    }
  } while (recieved != 1);
}
/*void stage_10()
  {
  forward = 0;
  back = 0;
  left = 1;
  right = 0;
  left_line = 0;
  while (stage == 10 && left_line < 5)
  {
    char readSrial = '\0';
    patrol();
    left_black_line_test();
    if ((digitalRead(8) == 1 && digitalRead(9) == 1) && (left_line == box_position[red_circle] || left_line == box_position[red_triangle] || left_line == box_position[red_square]))
    {
      send_move_data(0, 0, 0);
      Serial.println("stop");
      delay(300);
      Serial2.print('S');
      do
      {
        if (Serial2.available())
        {
          readSrial = Serial.read();
        }
      } while (readSrial != 'G'); //等待云台发送运动指令
    }
  }
  }*/
void left_black_line_test()
{
  if (digitalRead(8) == 1 && digitalRead(9) == 1)
  {
    if (left_line_last_state == 0)
    {
      left_line_last_state = 1;
      left_line++;
    }
  }
  else
  {
    left_line_last_state = 0;
  }
}
void right_black_line_test()
{
  if (digitalRead(4) == 1 && digitalRead(5) == 1)
  {
    if (right_line_last_state == 0)
    {
      right_line_last_state = 1;
      right_line++;
    }
  }
  else
  {
    right_line_last_state = 0;
  }
}
void back_black_line_test()
{

  /* Serial.print("D2state:");
    Serial.print(digitalRead(2));
    Serial.print("digitalRead(3):");
    Serial.print(digitalRead(3));*/
  if (D2state && D3state)
  {
    if (back_line_last_state == 0)
    {
      back_line_last_state = 1;
      back_line++;
    }
  }
  else
  {
    back_line_last_state = 0;
  }
}
void forward_black_line_test()
{
  if (digitalRead(7) == 1 && digitalRead(6) == 1)
  {
    if (forward_line_last_state == 0)
    {
      forward_line++;
      forward_line_last_state = 1;
    }
  }
  else
  {
    forward_line_last_state = 0;
  }
}
void forward_patrol()
{
  switch (stage)
  {
    case 2:
      move_forward_speed = 850;
      move_forward_turn_left_speed = 250;
      move_forward_turn_right_speed = 100;
      break;

    default:
      move_forward_speed = 1500;
      move_forward_turn_left_speed = 250;
      move_forward_turn_right_speed = 100;
      break;
  }
  if (flag)
  {
    move_forward_speed = 850;
    move_forward_turn_left_speed = 250;
    move_forward_turn_right_speed = 100;
  }
  if (digitalRead(7) == 0 && digitalRead(6) == 0 && forward)
    move_forward();
  if (digitalRead(7) == 1 && digitalRead(6) == 0 && forward)
    move_forward_left();
  if (digitalRead(7) == 0 && digitalRead(6) == 1 && forward)
    move_forward_right();
}

void back_patrol()
{
  switch (stage)
  {
    case 4:
      move_back_speed = 1500;
      move_back_turn_speed = 250;
      break;

    default:
      move_back_speed = 1000;
      move_back_turn_speed = 250;
      break;
  }
  if (digitalRead(2) == 0 && digitalRead(3) == 0 && back)
    move_back();
  if (digitalRead(2) == 1 && digitalRead(3) == 0 && back)
    move_back_right();
  if (digitalRead(2) == 0 && digitalRead(3) == 1 && back)
    move_back_left();
}
void left_patrol()
{
  switch (stage)
  {
    case 3:
      move_left_speed = 1000;
      move_left_turn_left_speed = 300; //以前300能走直
      move_left_turn_right_speed = 300;
      break;

    default:
      move_left_speed = 1000;
      move_left_turn_left_speed = 500;
      move_left_turn_right_speed = 300;
      break;
  }
  if (digitalRead(9) == 0 && digitalRead(8) == 0 && left)
  {
    move_left();
  }
  if (digitalRead(9) == 1 && digitalRead(8) == 0 && left)
  {
    move_left_left();
  }
  if (digitalRead(9) == 0 && digitalRead(8) == 1 && left)
  {
    move_left_right();
  }
}
void right_patrol()
{
  switch (stage)
  {
    case 1:
      move_right_speed = 1000;
      move_right_turn_left_speed = 250;
      move_right_turn_right_speed = 250;
      break;
    case 5:
      move_right_speed = 1500;
      move_right_turn_left_speed = 500;
      move_right_turn_right_speed = 500;
      break;
    default:
      move_right_speed = 1200;
      move_right_turn_left_speed = 500;
      move_right_turn_right_speed = 500;
      break;
  }

  if (digitalRead(4) == 0 && digitalRead(5) == 0 && right)
  {
    move_right();
  }
  if (digitalRead(4) == 1 && digitalRead(5) == 0 && right)
  {
    move_right_left();
  }
  if (digitalRead(4) == 0 && digitalRead(5) == 1 && right)
  {
    move_right_right();
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////底盘移动子函数//////////////////////////////////////////////////////////////////////////////

void patrol()
{
  if (forward)
  {
    Serial.println("向前巡线");
    forward_patrol();
  }
  if (back)
  {
    Serial.println("后退巡线");
    back_patrol();
  }
  if (left)
  {
    Serial.println("平移左巡线");
    left_patrol();
  }

  if (right)
  {
    Serial.println("平移右巡线");
    right_patrol();
  }
}

void send_move_data(uint16_t move_speed, int16_t move_angle, int16_t angular_velocity)
{
  uint8_t data[6];
  data[0] = (move_speed >> 8) & 0xFF;
  data[1] = move_speed & 0xFF;
  data[2] = (move_angle >> 8) & 0xFF;
  data[3] = move_angle & 0xFF;
  data[4] = (angular_velocity >> 8) & 0xFF;
  data[5] = angular_velocity & 0xFF;
  CAN.sendMsgBuf(0x208, 0, 8, data);
}
void move_forward()
{
  send_move_data(move_forward_speed, 0, 0); //前进
  Serial.println("前进");
}
void move_back()
{
  send_move_data(-move_back_speed, 0, 0); //后退
  Serial.println("后退");
}
void move_left()
{
  //send_move_data(-move_left_speed, 90, 0); //横向左
  send_move_data(1000, -88, -11);
  Serial.println("横向左");
}
void move_right()
{
  send_move_data(move_right_speed, 90, 0); //横向右
  Serial.println("横向右");
}
void move_forward_left()
{
  send_move_data(move_forward_speed, 0, -move_forward_turn_left_speed); //前进左转
  Serial.println("前进左转");
}
void move_forward_right()
{
  send_move_data(move_forward_speed, 0, move_forward_turn_right_speed); //前进右转
  Serial.println("前进右转");
}
void move_back_left()
{
  send_move_data(-move_back_speed, 0, -move_back_turn_speed); //后退左转
  Serial.println("后退左转");
}
void move_back_right()
{
  send_move_data(-move_back_speed, 0, move_back_turn_speed); //后退右转
  Serial.println("后退右转");
}
void move_left_left()
{
  send_move_data(-move_left_speed, 90, -move_left_turn_left_speed); //横向左左转
  Serial.println("横向左左转");
}
void move_left_right()
{
  send_move_data(-move_left_speed, 90, move_left_turn_right_speed); //横向左右转
  Serial.println("横向左右转");
}
void move_right_left()
{
  send_move_data(move_right_speed, 90, -move_right_turn_left_speed); //横向右左转
  Serial.println("横向右左转");
}
void move_right_right()
{
  send_move_data(move_right_speed, 90, move_right_turn_right_speed); //横向右右转
  Serial.println("横向右右转");
}
void STOP()
{
  Serial.println("进入STOP");
  if (digitalRead(4) == 1 && digitalRead(5) == 1 && right)
  {
    send_move_data(0, 0, 0);
    Serial.println("stop");
    delay(300);
  }
  if (digitalRead(2) == 1 && digitalRead(3) == 1 && back)
  {
    send_move_data(0, 0, 0);
    delay(300);
  }
  if (digitalRead(6) == 1 && digitalRead(7) == 1 && forward)
  {
    send_move_data(0, 0, 0);
    delay(300);
  }
  if (digitalRead(8) == 1 && digitalRead(9) == 1 && left)
  {
    send_move_data(0, 0, 0);
    delay(300);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////通讯///////////////////////////////////////////////////////////
