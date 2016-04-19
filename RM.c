/**0224:
#include <SetLCDClear.h>
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
	*try using andy's attack strategy in attackStrategy();
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
*/
#define STOP 360
#define TESTSPEED 60

#include <stdio.h>
#include <GetCompoI3.h>
#include <math.h>
#include "HardwareInfo.c"
#include <SetMotor.h>
#include <SetLCDFilledRectangle.h>
#include "JMLib.c"
#include <GetData.h>
#include <SetLCD5Char.h>
#include <GetCompassB.h>
#include <SetSysTime.h>
#include <GetSysTime.h>
#include <SetCentiS.h>
#include <GetAdUltrasound.h>
#include <stdbool.h>
#include <GetADScable10.h>
#include <GetTouchScreenX.h>
#include <SetLCDClear.h>
#include <GetRemoIR.h>

int eyePort;
int previousEyePort;
int positionI;
int speed;
int main(void)
{
	X2RCU_Init();//initialize the RCU;
	int angle=0;//the angle of compass;
	int direction=STOP;//the direcion robot goes,360 means stop;
	int display = 1;//indicate what to display
	int greyPort = 0;
	int pressed = 0;
	int targetAngle = 0;
	extern int speed;
	speed = 60;
	previousEyePort = eyePort;
	
	
	
	while (1){//forever running loop;
		pressed = GetTouchScreenX();
		if(pressed){
		
			pressed = GetTouchScreenX();
			if(!pressed){
				display=(display+1)%2;
			}
		}
		displayAll(display);
		
		
		eyePort = getEyePort(5,45,targetAngle);//get the port of the fly eye giving lower thread 5 and higher thread 60
		if(eyePort){
			speed = 60;
			direction = attackStrategy(eyePort,direction);
		}
		
		else{
			speed = 50;
			direction =backPosition();
		}
		greyPort = getGreyPort();
		if(greyPort){
			direction = whiteLineStrategy();
		}
		move(direction,speed,targetAngle);//give the direction and speed to <move>mothed in order to react
		previousEyePort = eyePort;//check the previous port in order to make advanced move
	}
}



int getShootTime(int lastShootTime){
	int output = lastShootTime;
	int fire = GetRemoIR(_FLAMEDETECT_fire_);
	int time = GetSysTime();
	if (time-lastShootTime>300&&fire<1800){
		output = time;
	}
	
	return output;
}

int getGreyPort(){
	/*a function to determine if the robot touches the white line;
	  *return the number of grey scale sensors touching the white line;
	  */
	
	
	int output = 0;
	int gFront = GetADScable10(_SCABLEAD_gFront_);
	int gInnerLeft = GetADScable10(_SCABLEAD_gInnerLeft_);
	int gInnerBack = GetADScable10(_SCABLEAD_gInnerBack_);
	int gInnerRight = GetADScable10(_SCABLEAD_gInnerRight_);
	int gOutterLeft = GetADScable10(_SCABLEAD_gOutterLeft_);
	int gOutterBack = GetADScable10(_SCABLEAD_gOutterBack_);
	int gOutterRight = GetADScable10(_SCABLEAD_gOutterRight_);
	
	if (gFront<1900||gInnerLeft<900||gInnerRight<1400||gInnerBack<1750){
		output = 1;
	}
	else if (gOutterLeft<1650){
		output = 2;
	}
	if (gOutterRight<1750){
		output = 3;
	}
	
	if (gOutterBack<1400){
		output = 4;
	}
	return output;
}
int whiteLineStrategy(int GP,int d){
	int direction=360;
	int startTime=GetSysTime();
	while(GetSysTime()-startTime<200){
		direction = backPosition();
		move(direction,15,0);
	}
	return d;
	
}


int getEyePort(int lowerThread,int higherThread){
	/**intake a lower threadshold and a higher threadshold;
	    *return the port which has the largest eye value
	    *return 0 when the value is smaller than the lower threadshold;
	    *return 1~14 when ball is far;
	    *return 15~28 when ball is close;
	    */
	extern int eyePort;
	int output=0;
	eyePort=0;
	int eyeValue = GetCompoI3( _COMPOUNDEYE3_leftEye_ ,9);
	int rightEyeValue = GetCompoI3( _COMPOUNDEYE3_rightEye_ ,9);
	if (eyeValue>lowerThread||rightEyeValue>lowerThread){
		if(eyeValue>rightEyeValue){
			eyePort =GetCompoI3(_COMPOUNDEYE3_leftEye_,8);
		}
		else{
			eyeValue = rightEyeValue;
			eyePort =GetCompoI3(_COMPOUNDEYE3_rightEye_,8)+7;
		}
		if (eyeValue>higherThread){
			eyePort+=14;
		}
	}
	//SetLCD5Char(0,100,eyeValue,YELLOW,BLACK);//display the value of the fly eye
	return eyePort;	
}







double degreeToRadian(int degree){
	return (degree*M_PI)/180;
}


void move(int d,int s,int targetAngle){
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
	double radianAngle;
	
	
	
	if (d<45){
		//set up the direction of each motor
		direction1 = 0;
		direction2 = 0;
		direction3 = 2;
		direction4 = 2;
		radianAngle= degreeToRadian(45-d);//calculate the angle needed to calculate the slower speed
		slowerSpeed = tan(radianAngle)*s;//calculate the slower speed in order to control direction of the robot
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
		radianAngle = degreeToRadian(d-45);
		slowerSpeed = tan(radianAngle)*s;
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
		radianAngle = degreeToRadian(135-d);
		slowerSpeed = tan(radianAngle)*s;
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
		radianAngle = degreeToRadian(d-135);
		slowerSpeed = tan(radianAngle)*s;
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
		radianAngle = degreeToRadian(225-d);
		slowerSpeed = tan(radianAngle)*s;
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
		radianAngle = degreeToRadian(d-225);
		slowerSpeed = tan(radianAngle)*s;
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
		radianAngle = degreeToRadian(315-d);
		slowerSpeed = tan(radianAngle)*s;
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
		radianAngle = degreeToRadian(d-315);
		slowerSpeed = tan(radianAngle)*s;
		speed1=s;
		speed2=slowerSpeed;
		speed3=s;
		speed4=slowerSpeed;
	}
	else if(d == STOP){
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
	if (abs(angleDif)>40){
		if (angleDif<0){
			//turn clockwise
			direction1=0;
			direction2 = 0;
			direction3=0;
			direction4=0;
			speed1=30;
			speed2=30;
			speed3=30;
			speed4=30;
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
	
	else if (abs(angleDif)>5){
		if(d==STOP){
			if (angle>180){
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
	if (p==1||p==14){
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
			output=170;
		}
		else if(p ==3){
			output = 180;
		}
		else if(p ==4){
			output=200;
		}
		else if(p ==5){
			output=220;
		}
		else if(p ==6){
			output=320;
		}
		else if(p ==7||p==8){
			output=0;
		}
		else if (p==9){
			output=40;
		}
		else if(p ==10){
			output=140;
		}
		else if(p ==11){
			output=160;
		}
		else if(p ==12){
			output=180;
		}
		else if (p==13){
			output=190;
		}
	return output;
}

int farStrategy(int p){
	int output = STOP;
	if (p==1||p==14){
		output= 210;
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
		output =300;
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
					else if (uBack<300){
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
						output = 0;
					}
					else if (uBack<700){
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
			output = STOP;
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

void displayAll(int i){
	
	if(i==1){
		int frontEyeValue;
		int backEyeValue;
		int frontEyePort;
		int backEyePort;
		int uLeft,uRight,uFront,uBack;
		int angle;
		int gLeft,gRight,gFront,gBack;
		
		
		angle = GetCompassB(_COMPASS_compass_);
		uLeft = GetAdUltrasound( _ADULTRASOUND_uLeft_);
		uRight = GetAdUltrasound( _ADULTRASOUND_uRight_);
		uFront = GetAdUltrasound(_ADULTRASOUND_uFront_);
		uBack = GetAdUltrasound(_ADULTRASOUND_uBack_);
		frontEyeValue = GetCompoI3( _COMPOUNDEYE3_leftEye_ ,9);
		backEyeValue = GetCompoI3( _COMPOUNDEYE3_rightEye_ ,9);
		frontEyePort =GetCompoI3(_COMPOUNDEYE3_leftEye_,8);		
		backEyePort =GetCompoI3(_COMPOUNDEYE3_rightEye_,8);
		
		SetLCD5Char( 0 ,0 ,frontEyeValue ,YELLOW ,BLACK );
		SetLCD5Char( 50 ,0 ,backEyeValue ,YELLOW ,BLACK );
		SetLCD5Char( 100 ,0 ,frontEyePort ,YELLOW ,BLACK );
		SetLCD5Char( 150 ,0 ,backEyePort ,YELLOW ,BLACK );
		
		SetLCD5Char( 0 ,20 ,uFront ,RED ,BLACK );
		SetLCD5Char( 50 ,20 ,uLeft ,RED ,BLACK );
		SetLCD5Char( 100 ,20 ,uRight ,RED ,BLACK );
		SetLCD5Char( 150 ,20 ,uBack ,RED ,BLACK );
		
		SetLCD5Char( 0 ,40 ,gFront ,BLUE ,BLACK );
		SetLCD5Char( 50 ,40 ,gLeft ,BLUE ,BLACK );
		SetLCD5Char( 100 ,40 ,gRight ,BLUE ,BLACK );
		SetLCD5Char( 150 ,40 ,gBack ,BLUE ,BLACK );
		
		SetLCD5Char( 0 ,60 ,angle ,GREEN ,BLACK );
		SetLCD5Char( 50 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 100 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 150 ,60 ,1 ,GREEN ,BLACK );
	}
	else if(i==2){
		SetLCD5Char( 0 ,0 ,1 ,YELLOW ,BLACK );
		SetLCD5Char( 50 ,0 ,1 ,YELLOW ,BLACK );
		SetLCD5Char( 100 ,0 ,1 ,YELLOW ,BLACK );
		SetLCD5Char( 150 ,0 ,1 ,YELLOW ,BLACK );
		
		SetLCD5Char( 0 ,20 ,1 ,RED ,BLACK );
		SetLCD5Char( 50 ,20 ,1 ,RED ,BLACK );
		SetLCD5Char( 100 ,20 ,1 ,RED ,BLACK );
		SetLCD5Char( 150 ,20 ,1 ,RED ,BLACK );
		
		SetLCD5Char( 0 ,40 ,1 ,BLUE ,BLACK );
		SetLCD5Char( 50 ,40 ,1 ,BLUE ,BLACK );
		SetLCD5Char( 100 ,40 ,1 ,BLUE ,BLACK );
		SetLCD5Char( 150 ,40 ,1 ,BLUE ,BLACK );
		
		SetLCD5Char( 0 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 50 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 100 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 150 ,60 ,1 ,GREEN ,BLACK );
	}
	else if(i==3){
		SetLCD5Char( 0 ,0 ,1 ,YELLOW ,BLACK );
		SetLCD5Char( 50 ,0 ,1 ,YELLOW ,BLACK );
		SetLCD5Char( 100 ,0 ,1 ,YELLOW ,BLACK );
		SetLCD5Char( 150 ,0 ,1 ,YELLOW ,BLACK );
		
		SetLCD5Char( 0 ,20 ,1 ,RED ,BLACK );
		SetLCD5Char( 50 ,20 ,1 ,RED ,BLACK );
		SetLCD5Char( 100 ,20 ,1 ,RED ,BLACK );
		SetLCD5Char( 150 ,20 ,1 ,RED ,BLACK );
		
		SetLCD5Char( 0 ,40 ,1 ,BLUE ,BLACK );
		SetLCD5Char( 50 ,40 ,1 ,BLUE ,BLACK );
		SetLCD5Char( 100 ,40 ,1 ,BLUE ,BLACK );
		SetLCD5Char( 150 ,40 ,1 ,BLUE ,BLACK );
		
		SetLCD5Char( 0 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 50 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 100 ,60 ,1 ,GREEN ,BLACK );
		SetLCD5Char( 150 ,60 ,1 ,GREEN ,BLACK );
	}
}


	