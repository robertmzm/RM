/**0224:
		*combine further() and closer() strategy into one function attackStrategy();
		*tried smooth turn between different direction. worked but notas good as ideal;
    *0226:
		*add turning into move() instead of doing it explicitly;
    *0304:
		*implement displayAll();
		*implement backPostion
    *0312:
		*implement wheels for function related for white line(did not work well);
		*inhence the turning functionality in move();
    *0315:
		*trying using andy's attack strategy in attackStrategy();
		*add some more documentry for functions;
		*define STOP as 360
    *0318:
		*new idea for white line;
    *0416:
        *lost all the files except for this only backup from 0318....damn...;
		*adding this text directly from notepad,does it work?;
		*just came back from notepad. yeah it works!!!;
		*implement getAngleDif
		*have to change AttackStrategy();
    *0418:
        *implement getShootTime();
        *start to use github in order to work on different device;
        *trying to make some changes from git;
        *making changes from mac!!!!!!!;
		*making changes from windows!!!!;
	*0419:
		*change all the direction in AttackStrategy();
		*enhance whitelineStratey();
		*implement getTargetAngle();
		*optimize getGreyPort():
			close gOutterLeft and gOutterBack when turning right;
			close gOutterRight and gOutterBack when turning left;
		*enhance whiteLineStrategy();
		*add some features to displayAll();
	*0420:
		*implement shoot();
		*change shoot time into a interval;
		*fixed bug in whiteLineStrategy;
	*0425:
		*fixed bug in main(), caused by wrong parameter.
	*0426:
		*just installed a atom plugin!!!it's soooo cooool;
	*0507:
		*change the threshold of the turning depends on the targetAngle;
		*flipped the fly eye on the left;
		*change some direction of closerStrategy;
		*optimize the display on the screen;
	*0508:
		*added comment;
		*displayAll() now have several pages to switch by using buttons;
		*found that using more sensors does take much more time per loop;
	*0509:
		*implemented logIn();
		*change displayAll() to screen();
		*change display to screenI;

*/
#define STOP 360
#define BLOCKED 361
#define TESTSPEED 70

#include <stdio.h>
#include <GetCompoI3.h>
#include <math.h>
#include "HardwareInfo.c"
#include <SetMotor.h>
#include <SetLCDFilledRectangle.h>
#include "JMLib.c"
#include <GetData.h>
#include <SetLCD5Char.h>
#include <SetLCDString.h>
#include <GetCompassB.h>
#include <SetSysTime.h>
#include <GetSysTime.h>
#include <SetCentiS.h>
#include <GetAdUltrasound.h>
#include <stdbool.h>
#include <GetADScable10.h>
#include <GetTouchScreenX.h>
#include <GetTouchScreenY.h>
#include <SetLCDClear.h>
#include <GetRemoIR.h>
#include <SetLED.h>
#include <GetButton1.h>
#include <GetButton2.h>
#include <GetButton3.h>
#include <SetLCDRectangle.h>
#include <SetLCDSolidCircle.h>

int positionI;
int speed;
int screenI;

int main(void)
{
	
	X2RCU_Init();//initialize the RCU;
	int angle=0;//the angle of compass;
	int direction=STOP;//the direcion robot goes,360 means stop;
	extern int screenI;//indicate what to display
	int greyPort = 0;
	int pressing = 0;//indicate if the button is being pressed
	int pressed = 0;//indicate if the button was pressed in the previous loop
	int targetAngle = 0;//the angle the robot wants to face
	int lowEyeThres = 5;//the threshold for the value of infrared sensors
	int highEyeThres = 80;//the treshold for far and close infrared ball
	int lastShootTime= -300;//the time of the last shot
	int eyePort = 0;
	int shooting = 0;//indicate if it is shooing or not

	extern int speed;

	screenI = 0;

	logIn();

	while (1){//forever running loop;

		/*detect if the first button is pressed in order to switch to
		  different pages;
		 */
		pressing = GetButton1();
		if(pressing ==0&&pressed == 1){//swich pages only when the button is released
			screenI=(screenI+1)%3;
			SetLCDClear(BLACK);
		}
		pressed = pressing;
		screen(screenI);//display everything


		eyePort = getEyePort(lowEyeThres,highEyeThres);//get the port of the fly eye giving lower thres 5 and higher thres 60

		if(eyePort){//ball is detected
			speed = TESTSPEED;
			direction = attackStrategy(eyePort,direction);
		}
		else{//no ball
			speed = 50;
			direction =backPosition();
		}

		targetAngle = getTargetAngle(targetAngle,eyePort);//calculate where the robot needs to face;

		lastShootTime = getShootTime(lastShootTime,eyePort);//determine if it is the time to shoot;
		shooting = shoot(lastShootTime);//shoot!!!! and get the state of the shot;

		greyPort = getGreyPort(targetAngle);//detect if the robot is touching the white line;
		if(greyPort){
			SetLED(_LED_shoot_,0);//turn of the solenoid because there is a loop inside;
			targetAngle = 0;//set target angle back to zero since white line is detected;
			direction = whiteLineStrategy(direction);//make sure the robot is going to the same direction as the function inside;
		}

		move(direction,speed,targetAngle,shooting);//give the direction and speed to move() in order to react;
	}
}

int shoot(int lastShootTime){
	/*intake the time to shoot;
	 *shoot if the difference of the time is small than the threshold;
	 *return the status of the solenoid;
	 */
	int timeDiff = GetSysTime()-lastShootTime;
	extern int screenI;

	if(screenI == 1){
		SetLCD5Char(100,120,timeDiff,RED,BLACK);
	}

	if(timeDiff<15){
		SetLED(_LED_shoot_,1);
		return 1;
	}
	else{
		SetLED(_LED_shoot_,0);
		return 0;
	}
}

int getTargetAngle(int previousTarget,int eyePort){
	/*a function to calculate which angle shoud the robot face;
	  *intake the previous ange the robot wanted to face;
	  *output the new angle the robot needs to face;
	  */
	int output = previousTarget;//make the previoud angle as the default output;
	if (eyePort<17||eyePort>25){//targetAngle goes back to 0 when the ball is in the back;
		output = 0;
	}
	/*need to make sure the previous angle is 0 so that the ultrasonic sensor is working correctly;
	 *only turns when the ball is at front and close to the robot;
	 */
	else if(previousTarget==0&&(eyePort==21||eyePort==22)){

		int uFront = GetAdUltrasound(_ADULTRASOUND_uFront_);
		int uLeft = GetAdUltrasound(_ADULTRASOUND_uLeft_);
		int uRight = GetAdUltrasound(_ADULTRASOUND_uRight_);
		int uBack = GetAdUltrasound(_ADULTRASOUND_uBack_);

		if(uLeft+uBack>1200){//nothing is blocking on the left and right;
			if(uBack>900&&uFront<1000){
				if(uLeft<550){
					output = 30;
				}
				else if(uRight<550){
					output = 330;
				}
			}
			else if(uBack<500&&uFront>800){
				if(uLeft<550){
					output = 330;
				}
				else if(uRight<550){
					output = 30;
				}
			}
		}
	}
	return output;

}

int getShootTime(int lastShootTime,int eyePort){
	/*intake the time of previous shot and the current eyePort;
	 *output the a new time if a shot is needed;
	 *output the previous shot time if no shot needed;
	 */
	int output = lastShootTime;
	int fire = GetRemoIR(_FLAMEDETECT_fire_);
	extern int screenI;

	if(screenI ==1){
		SetLCD5Char(50,120,fire,RED,BLACK);
	}
	int time = GetSysTime();
	if (time-lastShootTime>100&&fire<400&&(eyePort == 21|| eyePort ==22)){//shoot when ball is on the front and close enough;
		output = time;
	}

	return output;
}

int getGreyPort(int targetAngle){
	/*a function to determine if the robot touches the white line;
	  *return the number of grey scale sensors touching the white line;
	  *intake the angle the robot is facing;
	  *turn off some sensors if need;
	  */

	int output = 0;
	int gFront = GetADScable10(_SCABLEAD_gFront_);
	int gInnerLeft = GetADScable10(_SCABLEAD_gInnerLeft_);
	int gInnerBack = GetADScable10(_SCABLEAD_gInnerBack_);
	int gInnerRight = GetADScable10(_SCABLEAD_gInnerRight_);
	int gOutterLeft = GetADScable10(_SCABLEAD_gOutterLeft_);
	int gOutterBack = GetADScable10(_SCABLEAD_gOutterBack_);
	int gOutterRight = GetADScable10(_SCABLEAD_gOutterRight_);
	if(targetAngle==0){
		if (gFront<1900||gInnerLeft<900||gInnerRight<1500||gInnerBack<1850){
			output = 1;
		}
		else if (gOutterLeft<1700){
			output = 2;
		}
		else if (gOutterRight<1670){
			output = 3;
		}

		else if (gOutterBack<1350){
			output = 4;
		}
	}
	else if(targetAngle>180){
		if(gFront<1900||gInnerLeft<900||gInnerRight<1500||gInnerBack<1850||
			gOutterRight<1650){
			output = 1;
		}
	}
	else{
		if(gFront<1900||gInnerLeft<900||gInnerRight<1500||gInnerBack<1850||
			gOutterLeft<1700){
			output = 1;
		}
	}
	return output;
}
int whiteLineStrategy(int d){
	/*the function is called only when white line is detected;
	 *intake the direction the robot was going;
	 *output the direction the robot is going;
	 */


	int direction=d;
	int startTime=GetSysTime();


	while(GetSysTime()-startTime<100&&direction!=STOP){
		direction = backPosition();
		move(direction,55,0);
	}
	return direction;
}


int getEyePort(int lowerThres,int higherThres){
	/**intake a lower threshold and a higher threshold;
	    *return the port which has the largest eye value
	    *return 0 when the value is smaller than the lower threshold;
	    *return 1~14 when ball is far;
	    *return 15~28 when ball is close;
	    */
	int eyePort;
	int output=0;
	eyePort=0;
	int eyeValue = GetCompoI3( _COMPOUNDEYE3_leftEye_ ,9);
	int rightEyeValue = GetCompoI3( _COMPOUNDEYE3_rightEye_ ,9);
	if (eyeValue>lowerThres||rightEyeValue>lowerThres){
		if(eyeValue>rightEyeValue){
			eyePort =8-GetCompoI3(_COMPOUNDEYE3_leftEye_,8);
		}
		else{
			eyeValue = rightEyeValue;
			eyePort =GetCompoI3(_COMPOUNDEYE3_rightEye_,8)+7;
		}
		if (eyeValue>higherThres){
			eyePort+=14;
		}
	}
	return eyePort;
}



double degreeToRadian(int degree){
	return (degree*M_PI)/180;
}


void move(int d,int s,int targetAngle,int shooting){
	/**A method to control the motor.
	    *Input the direction and the percentage of power the robot wants to go.
	    *No need to calculate the individual speed any more!
	    *the range of direction 0<=d<360
	    *361means turn clockwise,362 means turn counter-clockwise
	    */

	int direction1,direction2,direction3,direction4;//direction of each motor.
	int speed1,speed2,speed3,speed4;//speed of each motor.
	int slowerSpeed;//the speed for the slower motor in order to control the direction.
	int angle,angleDif;
	double radian;



	if (d<45){
		//set up the direction of each motor
		direction1 = 0;
		direction2 = 0;
		direction3 = 2;
		direction4 = 2;
		radian= degreeToRadian(45-d);//calculate the angle needed to calculate the slower speed
		slowerSpeed = tan(radian)*s;//calculate the slower speed in order to control direction of the robot
		//motor 2 and motor 4 go for full speed and motor 1 and motor 3 are slower
		speed1=slowerSpeed;
		speed2=s;
		speed3=slowerSpeed;
		speed4=s;
	}
	else if(d<90){
		direction1 = 2;
		direction2 = 0;
		direction3 = 0;
		direction4 = 2;
		radian = degreeToRadian(d-45);
		slowerSpeed = tan(radian)*s;
		speed1=slowerSpeed;
		speed2=s;
		speed3=slowerSpeed;
		speed4=s;
	}
	else if(d<135){
		direction1 = 2;
		direction2 = 0;
		direction3 = 0;
		direction4 = 2;
		radian = degreeToRadian(135-d);
		slowerSpeed = tan(radian)*s;
		speed1=s;
		speed2=slowerSpeed;
		speed3=s;
		speed4=slowerSpeed;
	}
	else if(d<180){
		direction1 = 2;
		direction2 = 2;
		direction3 = 0;
		direction4 = 0;
		radian = degreeToRadian(d-135);
		slowerSpeed = tan(radian)*s;
		speed1=s;
		speed2=slowerSpeed;
		speed3=s;
		speed4=slowerSpeed;
	}
	else if (d<225){
		direction1 = 2;
		direction2 = 2;
		direction3 = 0;
		direction4 = 0;
		radian = degreeToRadian(225-d);
		slowerSpeed = tan(radian)*s;
		speed1=slowerSpeed;
		speed2=s;
		speed3=slowerSpeed;
		speed4=s;

	}
	else if(d<270){
		direction1 = 0;
		direction2 = 2;
		direction3 = 2;
		direction4 = 0;
		radian = degreeToRadian(d-225);
		slowerSpeed = tan(radian)*s;
		speed1=slowerSpeed;
		speed2=s;
		speed3=slowerSpeed;
		speed4=s;
	}
	else if(d<315){
		direction1 = 0;
		direction2 = 2;
		direction3 = 2;
		direction4 = 0;
		radian = degreeToRadian(315-d);
		slowerSpeed = tan(radian)*s;
		speed1=s;
		speed2=slowerSpeed;
		speed3=s;
		speed4=slowerSpeed;
	}
	else if(d<360){
		direction1 = 0;
		direction2 = 0;
		direction3 = 2;
		direction4 = 2;
		radian = degreeToRadian(d-315);
		slowerSpeed = tan(radian)*s;
		speed1=s;
		speed2=slowerSpeed;
		speed3=s;
		speed4=slowerSpeed;
	}
	else if(d == STOP||d == BLOCKED){
		//stop
		direction1=1;
		direction2 = 1;
		direction3=1;
		direction4=1;
		speed1=s;
		speed2=s;
		speed3=s;
		speed4=s;
	}

	/**check the robot is facing the right direction;
	    *if the robot is shifted between 5 to 15 degree,
	      change each wheel's speed slightly in order to correct the direction;
	    *if the robot is shifted more then 15 degree,
	      change the direction by spinning;
	    */


	angleDif = getAngleDif(targetAngle);


	int angleThres;

	if(targetAngle == 0){
		angleThres = 20;//turn sharply when facing back
	}
	else{
		angleThres = 40;//turn smoothly when turn shooting;
	}

	if (abs(angleDif)>angleThres&&shooting!=1){
		if (angleDif<0){
			//turn clockwise
			direction1=0;
			direction2 = 0;
			direction3=0;
			direction4=0;
			speed1=16;
			speed2=16;
			speed3=16;
			speed4=16;
		}
		else{
			//turn counter-clockwise
			direction1=2;
			direction2 = 2;
			direction3=2;
			direction4=2;
			speed1=30;
			speed2=30;
			speed3=30;
			speed4=30;
		}
	}
	else if (abs(angleDif)>8){
		if(d==STOP||d==BLOCKED){
			if (angleDif<0){
				direction1 = 0;
				direction2 = 0;
				direction3 = 0;
				direction4 = 0;
				speed1 = 15;
				speed2 = 15;
				speed3 = 15;
				speed4 = 15;
			}
			else{
				direction1 = 2;
				direction2 = 2;
				direction3 = 2;
				direction4 = 2;
				speed1 = 15;
				speed2 = 15;
				speed3 = 15;
				speed4 = 15;
			}
		}
		else{
			speed1 = direction1==0?speed1-angleDif:speed1+angleDif;
			speed2 = direction2==0?speed2-angleDif:speed2+angleDif;
			speed3 = direction3==0?speed3-angleDif:speed3+angleDif;
			speed4 = direction4==0?speed4-angleDif:speed4+angleDif;
		}
	}

	speed1 = checkSpeed(speed1);
	speed2 = checkSpeed(speed2);
	speed3 = checkSpeed(speed3);
	speed4 = checkSpeed(speed4);

	extern int screenI;
	if(screenI==1){
		SetLCD5Char(0,80,speed2,WHITE,BLACK);
		SetLCD5Char(150,80,speed3,WHITE,BLACK);
		SetLCD5Char(0,100,speed1,WHITE,BLACK);
		SetLCD5Char(150,100,speed4,WHITE,BLACK);


		SetLCD5Char(0,120,d,CYAN,BLACK);
		SetLCD5Char(150,40,angleDif,GREEN,BLACK);
	}


	SetMotor(_MOTOR_M1_,direction1,speed1);
	SetMotor(_MOTOR_M2_,direction2,speed2);
	SetMotor(_MOTOR_M3_,direction3,speed3);
	SetMotor(_MOTOR_M4_,direction4,speed4);
}

int attackStrategy(int p,int previousDirection){
	/**intake the port which has the largest value;
	    *output the direction the robot should go;
	    *the strategy works when ball is both far or close;
	    */
	if (p<15){
		return farStrategy(p);
	}
	else{
		return closeStrategy(p-14);
	}
}

int closeStrategy(int p){
	int output;
	int uBack = GetAdUltrasound(_ADULTRASOUND_uBack_);

	if(uBack<200&&(p<5||p>10)){
		if(p<7){
			output = 270;
		}
		else{
			output = 90;
		}
	}
	else if (p==1||p==14){
		int uLeft = GetAdUltrasound(_ADULTRASOUND_uLeft_);
		int uRight = GetAdUltrasound(_ADULTRASOUND_uRight_);


		if(uLeft>uRight){
			output = 250;
		}
		else{
			output = 110;
		}
	}
	else if(p ==2){
		output=180;
	}
	else if(p ==3){
		output = 210;
	}
	else if(p ==4){
		output=240;
	}
	else if(p ==5){
		output=300;
	}
	else if(p ==6){
		output=330;
	}
	else if(p ==7||p==8){
		output=0;
	}
	else if (p==9){
		output=30;
	}
	else if(p ==10){
		output=60;
	}
	else if(p ==11){
		output=120;
	}
	else if(p ==12){
		output=150;
	}
	else if (p==13){
		output=180;
	}
	return output;
}

int farStrategy(int p){
	int output = STOP;
	if (p==1||p==14){
		int uLeft = GetAdUltrasound(_ADULTRASOUND_uLeft_);
		int uRight = GetAdUltrasound(_ADULTRASOUND_uRight_);
		if(uLeft>uRight){
			output= 210;
		}
		else{
			output = 150;
		}
	}
	else if(p ==2){
		output= 180;
	}
	else if(p ==3){
		output= 210;
	}
	else if(p ==4){
		output= 240;
	}
	else if(p ==5){
		output= 270;
	}
	else if(p ==6){
		output =280;
	}
	else if(p ==7||p==8){
		output= 0;
	}
	else if(p ==9){
		output= 60;
	}
	else if(p ==10){
		output= 90;
	}
	else if(p ==11){
		output= 120;
	}
	else if(p ==12){
		output= 150;
	}
	else if(p ==13){
		output= 180;
	}
	return output;
}

int backPosition(){
	/*called when the ball is not detected
	  *no input for now;
	  *output the direction needed to go back to the right location
	  */
	int uLeft,uRight,uFront,uBack;
	uLeft = GetAdUltrasound( _ADULTRASOUND_uLeft_);
	uRight = GetAdUltrasound( _ADULTRASOUND_uRight_);
	uFront = GetAdUltrasound(_ADULTRASOUND_uFront_);
	uBack = GetAdUltrasound(_ADULTRASOUND_uBack_);
	int output = STOP;
	if(uLeft+uRight>1000){
		//left and right are not blocked;

		if(uFront+uBack>800){
			/*front and back are not blcoked;
			  *all open;
			  /////////////////////////////////////
			  //			//			//			//
			  //		1  	//		2	//		3	//
			  //			//			//			//
			  //			//			//			//
			  /////////////////////////////////////
			  //			//			//			//
			  //		4	//		5	//		6	//
			  //			//			//			//
			  //			//			//			//
			  /////////////////////////////////////
			  //			//			//			//
			  //			//			//			//
			  //		7	//		8	//		9	//
			  //			//			//			//
			  //			//			//			//
			  /////////////////////////////////////
			*/
			if(uLeft<uRight){
				//using left
				if(uLeft<500){
					//pos 1,4,7
					if (uBack<350){
						//pos7
						output = 30;
					}
					else if (uBack<550){
						//pos4
						output = 90;
					}
					else{
						//pos1
						output = 150;
					}

				}
				else if(uLeft<1100){
					//pos 2,5,8
					if (uBack<200){
						//pos8
						output = 0;
					}
					else if (uBack<350){
						//pos5
						output = STOP;
					}
					else{
						//pos2
						output = 180;
					}
				}
				else{
					//pos 3,6,9
					if (uBack<350){
						output = 330;
					}
					else if (uBack<550){
						output = 270;
					}
					else{
						output = 210;
					}
				}
			}
			else{
				//using right
				if(uRight<500){
					//pos 3,6,9
					if (uBack<400){
						output = 330;
					}
					else if (uBack<1000){
						output = 270;
					}
					else{
						output = 210;
					}
				}
				else if(uRight<1100){
					//pos 2,5,8
					if (uBack<200){
						//pos8
						output = 350;
					}
					else if (uBack<500){
						//pos5
						output = STOP;
					}
					else{
						//pos2
						output = 180;
					}
				}
				else{
					//pos 1,4,7
					if (uBack<400){
						//pos7
						output = 30;
					}
					else if (uBack<1000){
						//pos4
						output = 90;
					}
					else{
						//pos1
						output = 150;
					}
				}
			}
		}
		else{
			/*front and back are blocked;
			//////////////////////////////////////
			//			//			//			//
			//			//			//			//
			//		1	//		2	//		3	//
			//			//			//			//
			//			//			//			//
			//////////////////////////////////////
			*/
			if(uLeft<uRight){
				//using left
				if(uLeft<600){
					//pos 1
					output = 90;
				}
				else if(uLeft<1200){
					//pos 2
					output = STOP;
				}
				else{
					//pos 3
					output = 270;
				}
			}
			else{
				//using right
				if(uRight<600){
					//pos 3
					output = 270;
				}
				else if(uRight<1200){
					//pos 2
					output = STOP;
				}
				else{
					//pos 1
					output = 90;
				}
			}
		}
	}
	else{
		//left and right are blocked;
		if(uFront +uBack>600){
			/*front and back are not blocked;
			//////////////////
			//				//
			//		1		//
			//				//
			//				//
			//////////////////
			//				//
			//		2		//
			//				//
			//				//
			//////////////////
			//				//
			//		3		//
			//				//
			//				//
			//////////////////
			*/
			if(uBack<uFront && uBack<400){
				//pos3
				output = 0;
			}
			else if((uFront<uBack && uFront<400)||uBack>500){
				//pos1
				output = 180;
			}
			else{
				//pos2
				output = STOP;
			}

		}
		else{
			//all direction are blcoked;
			output = BLOCKED;
		}
	}
	return output;
}


int maxMin(int n,int max, int min){
	if (n>max){
		n=max;
	}
	else if (n<min){
		n=min;
	}
	return n;
}

int checkSpeed(int speed){
	return maxMin(speed,100,0);
}

int getAngleDif(int target){
	int output = 0;
	int current = GetCompassB(_COMPASS_compass_);
	if(target<180){
		if(current>target+180){
			output = -360+current-target;
		}
		else{
			output = current-target;
		}
	}
	else{
		if(current>target-180){
			output = current-target;
		}
		else{
			output = 360-target+current;
		}
	}

	return output;
}

void screen(int i){

	if(i==1){
		int leftEyeValue;
		int rightEyeValue;
		int leftEyePort;
		int rightEyePort;
		int uLeft,uRight,uFront,uBack;
		int angle;
		int gFront,gInnerLeft,gOutterLeft,gInnerRight,gOutterRight,gInnerBack,gOutterBack;



		angle = GetCompassB(_COMPASS_compass_);
		uLeft = GetAdUltrasound( _ADULTRASOUND_uLeft_);
		uRight = GetAdUltrasound( _ADULTRASOUND_uRight_);
		uFront = GetAdUltrasound(_ADULTRASOUND_uFront_);
		uBack = GetAdUltrasound(_ADULTRASOUND_uBack_);
		leftEyeValue = GetCompoI3( _COMPOUNDEYE3_leftEye_ ,9);
		rightEyeValue = GetCompoI3( _COMPOUNDEYE3_rightEye_ ,9);
		leftEyePort =8-GetCompoI3(_COMPOUNDEYE3_leftEye_,8);
		rightEyePort =GetCompoI3(_COMPOUNDEYE3_rightEye_,8);
		gFront = GetADScable10(_SCABLEAD_gFront_);
		gInnerLeft = GetADScable10(_SCABLEAD_gInnerLeft_);
		gOutterLeft = GetADScable10(_SCABLEAD_gOutterLeft_);
		gInnerRight = GetADScable10(_SCABLEAD_gInnerRight_);
		gOutterRight = GetADScable10(_SCABLEAD_gOutterRight_);
		gInnerBack = GetADScable10(_SCABLEAD_gInnerBack_);
		gOutterBack = GetADScable10(_SCABLEAD_gOutterBack_);

		SetLCD5Char(0,40,angle,GREEN,BLACK);

		SetLCD5Char( 0 ,0 ,leftEyeValue ,YELLOW ,BLACK );
		SetLCD5Char( 50 ,0 ,rightEyeValue ,YELLOW ,BLACK );
		SetLCD5Char( 100 ,0 ,leftEyePort ,YELLOW ,BLACK );
		SetLCD5Char( 150 ,0 ,rightEyePort ,YELLOW ,BLACK );

		SetLCD5Char( 0 ,20 ,uFront ,RED ,BLACK );
		SetLCD5Char( 50 ,20 ,uLeft ,RED ,BLACK );
		SetLCD5Char( 100 ,20 ,uRight ,RED ,BLACK );
		SetLCD5Char( 150 ,20 ,uBack ,RED ,BLACK );

		SetLCD5Char( 70 ,40 ,gFront ,BLUE ,BLACK );
		SetLCD5Char( 0 ,60 ,gOutterLeft ,BLUE ,BLACK );
		SetLCD5Char( 50 ,60 ,gInnerLeft ,BLUE ,BLACK );
		SetLCD5Char( 100 ,60 ,gInnerRight ,BLUE ,BLACK );
		SetLCD5Char( 150 ,60 ,gOutterRight ,BLUE ,BLACK );
		SetLCD5Char( 70 ,80 ,gInnerBack ,BLUE ,BLACK );
		SetLCD5Char( 70 ,100 ,gOutterBack ,BLUE ,BLACK );
	}
	else if (i==2){
	}
}

void logIn(){
	
	int count = 0;
	int password[] = {1,3,1,2,1,2};
	int digit = 0;
	drawRM(0);
	while(count<6){
	
		digit = getCode();
		
		if(digit == password[count]){
			
			count++;
		}
		else{
			count = 0;
		}
		SetLCD5Char(0,30,count,BLACK,BLACK);
	}
	
	drawRM(1);
	
}

void drawRM(int i){
	SetLCDClear(BLACK);
	int color = RED;
	if(i==1){
		color = GREEN;
	}
	SetLCDString(100,75,"RM",color,BLACK);
		
}

int getCode(){
	int pressing1,pressing2,pressing3;
	int pressed1,pressed2,pressed3;
	while(1){
		pressing1 = GetButton1();
		pressing2 = GetButton2();
		pressing3 = GetButton3();
		if(pressing1 ==0&&pressed1 == 1){
			return 1;
		}
		if(pressing2 ==0&&pressed2 == 1){
			return 2;
		}
		if(pressing3 ==0&&pressed3 == 1){
			return 3;
		}
		pressed1 = pressing1;
		pressed2 = pressing2;
		pressed3 = pressing3;
	}
}


\
void testShooting(){
	int eyePort = 21;
	int lastShootTime = -300;
	while(1){
		lastShootTime = getShootTime(lastShootTime,eyePort);
		shoot(lastShootTime);
	}
}
