/*
# File "guardbot.c"
# Copyright (c) 2009 M Bark
#    E-mail: mikbar at kth dot se
#
#    modified by: Jorge Mir�, Lyudmilla Gerlakh, Mikael Bark, Paul Hill, Dimitrios Tachtsioglou
# 
#    This file is part of the VR 09 system.
# 
#    VR09 is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
# 
#    VR09 is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
#
# File version: 1.0
# VR09 version: 1.0
#
# Modification history for this file:
# v1.0  (2009-05-15) First released version.
#
################################################################
*/

#include <system.h>
#include<stdlib.h>

//Konstanter f�r k�rriktningar
#define turn_right      0
#define turn_left       1
#define drive_forward   2
#define drive_backward  3
#define stop            4

#define ledLength       100000 //Tid f�r varje gr�n led

typedef unsigned int uint;

//Deklaration f�r l�nkad lista
struct driveNode {
    unsigned char driveState;
    uint driveTime;
    
    struct driveNode* next;
};

//driveList anv�nds f�r att definiera en sekvens k�rinstruktioner
struct driveNode* driveList;
int listDelay, listTime;

unsigned long outportTime[18];
uint outportLength[18];
uint outportState[18];

unsigned long inportTime[18];
uint inportLength[18];
uint inportState[18];

char buttonState[5];
char driveStates[5];
int larmState;

int pirTime;

void clearDriveList() {
    struct driveNode* tempNode;
    
    while(1) {
        tempNode = driveList->next;
    
        free(driveList);
        driveList = NULL;
    
        if(tempNode != NULL)
            driveList = tempNode;
        else
            break;
    } 
}

/*
 * initialize - S�tt alla variabler till sitt standardv�rde. Detta har som funktion att
 * nollst�lla roboten vid uppstart och omstart.
 */
void initialize() {
    int i;
    for(i=0; i<18; i++) {
        outportLength[i] = 0;
        outportState[i] = 0;
        outportTime[i] = 0;
        
        inportState[i] = 0;
        inportLength[i] = 50000;
        inportTime[i] = 0;
    }
  
    for(i=0; i<5; i++) {
        buttonState[i] = 0;

        signaloff(DE2_PIO_JP1_OUT1_5_BASE, i);
        signaloff(DE2_PIO_JP1_OUT3_5_BASE, i);     
    }
    
    for(i=0; i<8; i++) {
        signaloff(DE2_PIO_JP1_OUT2_8_BASE, i);        
    }
    
    driveList = malloc(sizeof (*driveList));

    (*driveList).driveState = drive_forward;
    (*driveList).driveTime = -1;
    (*driveList).next = NULL;
  
    larmState = 0;
    
    listDelay=0;
    listTime=0;
    
    pirTime = 0;
    
    clearSignals();
    clearGlobaltime();
}

/*
 * detectButtons - K�nn av switch adresserna efter knappl�ge. De tv� knappar som anv�nds �r robotens on/off l�ge samt
 * siren kontroll.
 */
void detectButtons() {
    buttonState[0] = signalread(DE2_PIO_TOGGLES18_BASE, 0); 
    buttonState[1] = signalread(DE2_PIO_TOGGLES18_BASE, 1);
    
    if(!buttonState[0]) {
        initialize();
    }
}

int driveInstructionButtons(){

    if(signalread(DE2_PIO_TOGGLES18_BASE, 17)) return drive_forward;
    if(signalread(DE2_PIO_TOGGLES18_BASE, 16)) return drive_backward;
    if(signalread(DE2_PIO_TOGGLES18_BASE, 15)) return stop;
    if(signalread(DE2_PIO_TOGGLES18_BASE, 14)) return turn_left;
    if(signalread(DE2_PIO_TOGGLES18_BASE, 13)) return turn_right;
}

/*
 * sendPulse - Anv�nds f�r att skicka puls till samtliga utportar. Argument anger hur
 * l�nge pulsen ska vara h�g respektive l�g samt vilken port signalen skickas till.
 */
void sendPulse(int highLength, int lowLength, int port) {
   int adress;
   int addressPort;
    if(port <=4){
        addressPort = port;
         adress = DE2_PIO_JP1_OUT1_5_BASE;
    }
    else if(port > 4 && port < 13){
        addressPort = port - 5;
        adress = DE2_PIO_JP1_OUT2_8_BASE;
    }
    else {
         addressPort = port - 13;
         adress = DE2_PIO_JP1_OUT3_5_BASE;
    }
    
    if(outportState[port]) {
        outportLength[port] = highLength;
        signalon(adress, addressPort);
    }
    else {
        outportLength[port] = lowLength;
        signaloff(adress, addressPort);
    }
    
    outportState[port] = !outportState[port];
}

/* driveFunction - Funktion f�r att k�ra i olika riktningar. Best�mmer med en case sats
 * vilken riktning bot ska k�ra i. Detta best�mms genom att skicka olika l�nga signaler
 * till servo.
 */
void driveFunction(int currentTime, int driveState) {
    switch(driveState) {
        //V�nd h�ger
        case 0:
            if((currentTime - outportTime[0]) > outportLength[0]) {
                outportTime[0] = currentTime;
                
                //Skicka puls p� out port (servo 1)
                sendPulse(200, 100, 0);
            }
            if((currentTime - outportTime[1]) > outportLength[1]) {
                outportTime[1] = currentTime;
                
                 //Skicka puls p� out port (servo 2)
                sendPulse(200, 100, 1);
            }
            break;
        //V�nd v�nster
        case 1:
            if((currentTime - outportTime[0]) > outportLength[0]) {
                outportTime[0] = currentTime;
                
                sendPulse(100, 100, 0);
            }
            if((currentTime - outportTime[1]) > outportLength[1]) {
                outportTime[1] = currentTime;
                
                sendPulse(100, 100, 1);
            }
            break;
        //K�r framm�t
        case 2:
            if((currentTime - outportTime[0]) > outportLength[0]) {
                outportTime[0] = currentTime;
                
                sendPulse(200, 100, 0);
            }
            if((currentTime - outportTime[1]) > outportLength[1]) {
                outportTime[1] = currentTime;
                
                sendPulse(100, 100, 1);
            }
            break;
        //K�r bak�t
        case 3:
            if((currentTime - outportTime[0]) > outportLength[0]) {
                outportTime[0] = currentTime;
                
                sendPulse(100, 100, 0);
            }
            if((currentTime - outportTime[1]) > outportLength[1]) {
                outportTime[1] = currentTime;
                
                sendPulse(200, 100, 1);
            }
            break;
        //St� still
        case 4:
            break;
        default:
            break;
    }
}

/*
 * sendPingPulse - skickar puls f�r samtliga pingsensorer.
 */
void sendPingPulse(int currentTime) {
    if((currentTime - outportTime[2]) > outportLength[2]) {
        outportTime[2] = currentTime;
        
        sendPulse(1, 4500, 2);
    }
    if((currentTime - outportTime[3]) > outportLength[3]) {
        outportTime[3] = currentTime;
        
        sendPulse(1, 4500, 3);
    }
    if((currentTime - outportTime[5]) > outportLength[5]) {
        outportTime[5] = currentTime;
        
        sendPulse(1, 4500, 5);
    }
}

/*
 * measurePing - Funktion f�r att m�ta signaler f�r pingsensorerna. Nuvarande tid avl�ses
 * n�r signal fr�n sensorerna g�r h�g och unders�ker tidsskillnaden n�r signalen g�r l�g.
 */
void measurePing(int currentTime, int* inportLength, int* inportLength2, int* inportLength4){
    if(inportState[0] ^ signalread(DE2_PIO_JP1_IN1_5_BASE, 0)) {
        inportState[0] = !inportState[0];
        
        if(inportState[0]) {
            inportTime[0] = currentTime;
        }
        else {
            *inportLength = currentTime - inportTime[0];
        }
    }
    
    if(inportState[1] ^ signalread(DE2_PIO_JP1_IN1_5_BASE, 1)) {
        inportState[1] = !inportState[1];
        
        if(inportState[1]) {
            inportTime[1] = currentTime;
        }
        else {
            *inportLength2 = currentTime - inportTime[1];
        }
    }

    if(inportState[3] ^ signalread(DE2_PIO_JP1_IN1_5_BASE, 3)) {
        inportState[3] = !inportState[3];
        
        if(inportState[3]) {
            inportTime[3] = currentTime;
        }
        else {
            *inportLength4 = currentTime - inportTime[3];
        }
    }

}

/*
 * pingDetect_obstacles - funktion ansvarig f�r att undvika hinder. N�r l�ga v�rden
 * p� pingsensorerna uppt�cks s� l�ggs en ny sekvens k�rinstruktioner in.
 */
void pingDetect_obstacles(int inportLength, int inportLength2, int inportLength4) {
    if((inportLength < 150)) {
        signalon(DE2_PIO_REDLED18_BASE, 10);
        int delayLength = 1000*(rand() % 25 + 50);
        
        //clearDriveList();
        
        listDelay = 0;
        
        struct driveNode* firstNode = malloc(sizeof (*driveList));
        driveList = firstNode;

        (*driveList).driveState = turn_left;
        (*driveList).driveTime = delayLength;
        (*driveList).next = malloc(sizeof (*driveList));
        
        driveList = driveList->next;

        (*driveList).driveState = stop;
        (*driveList).driveTime = 600000;
        (*driveList).next = malloc(sizeof (*driveList));
    
        driveList = driveList->next;
    
        (*driveList).driveState = drive_forward;
        (*driveList).driveTime = -1;
        (*driveList).next = NULL;
        
        driveList = firstNode;
    }
    else if((inportLength2 < 150)) {
        signalon(DE2_PIO_REDLED18_BASE, 13);
        int delayLength = 1000*(rand() % 25 + 50);
        
        //clearDriveList();
        
        listDelay = 0;
        
        struct driveNode* firstNode = malloc(sizeof (*driveList));
        driveList = firstNode;

        (*driveList).driveState = turn_right;
        (*driveList).driveTime = delayLength;
        (*driveList).next = malloc(sizeof (*driveList));
        
        driveList = driveList->next;

        (*driveList).driveState = stop;
        (*driveList).driveTime = 600000;
        (*driveList).next = malloc(sizeof (*driveList));
    
        driveList = driveList->next;
    
        (*driveList).driveState = drive_forward;
        (*driveList).driveTime = -1;
        (*driveList).next = NULL;
        
        driveList = firstNode;
    }
    if((inportLength4 < 200)) {
        signalon(DE2_PIO_REDLED18_BASE, 10);
        int delayLength = 1000*(rand() % 25 + 50);
        
        //clearDriveList();
        
        listDelay = 0;
        
        struct driveNode* firstNode = malloc(sizeof (*driveList));
        driveList = firstNode;

        (*driveList).driveState = drive_backward;
        (*driveList).driveTime = 50000;
        (*driveList).next = malloc(sizeof (*driveList));
      
        driveList = driveList->next;

        (*driveList).driveState = turn_right;
        (*driveList).driveTime = delayLength;
        (*driveList).next = malloc(sizeof (*driveList));
        
        driveList = driveList->next;

        (*driveList).driveState = stop;
        (*driveList).driveTime = 600000;
        (*driveList).next = malloc(sizeof (*driveList));
    
        driveList = driveList->next;
    
        (*driveList).driveState = drive_forward;
        (*driveList).driveTime = -1;
        (*driveList).next = NULL;
        
        driveList = firstNode;
    }
    else {
        signaloff(DE2_PIO_REDLED18_BASE, 10);
        signaloff(DE2_PIO_REDLED18_BASE, 13);
        signaloff(DE2_PIO_REDLED18_BASE, 15);
    }    
 
    
}

/*
 * pingDetect_intruder_PIR - funktion ansvarig f�r att l�sa av PIR r�relsedetektorn.
 * N�r signal g�r l�g s� sl�s larm p� och stannar roboten permanent tills omstart sker.
 */
void pingDetect_intruder_PIR(int currentTime){
    
    if(!signalread(DE2_PIO_JP1_IN1_5_BASE, 4)) {
        larmState = 1;
        
        listDelay = 0;
        
        //Stanna roboten efter larm
        /*driveList = malloc(sizeof (*driveList));

        (*driveList).driveState = stop;
        (*driveList).driveTime = -1;
        (*driveList).next = NULL;*/
    }
}

/*
 * Larm - ansvarig f�r att aktivera olika larmindikationer (kamera, led, siren).
 */
void larm(int currentTime){
    
    //Larmstate signalerar om larmet ska sl�s p�
    if(larmState) {
        if((currentTime - outportTime[15]) > outportLength[15]) {
            outportTime[15] = currentTime;
            sendPulse(100000,100000 , 15); //Skicka puls till kamera
        }     
        
        signalon(DE2_PIO_JP1_OUT3_5_BASE, 3); //Sl� p� signal till LED
    }
    else {
        signaloff(DE2_PIO_JP1_OUT3_5_BASE, 2); //Sl� av kamera
        signaloff(DE2_PIO_JP1_OUT3_5_BASE, 3); //Sl� av LED
        signaloff(DE2_PIO_JP1_OUT3_5_BASE, 4); //Sl� av Siren
    }
    
    //Larmstate ska bara sl� p� siren om switch 1 �r p�
    if(larmState && buttonState[1]) {
        if((currentTime - outportTime[17]) > outportLength[17]) {
            outportTime[17] = currentTime;
            sendPulse(10000,10000 , 17); //Skicka puls till siren
        }
    }
    else if(larmState && !buttonState[1]) {
        signaloff(DE2_PIO_JP1_OUT3_5_BASE, 4);
    }
}

/*
 * mainLoop - Huvudfunktion f�r roboten. Best�r av en kontinuerlig sekvens av
 * funktionsanrop. 
 */
void mainLoop() {
    unsigned long currentTime;
    int driveState;
    driveState = stop;
    while(1) {
        currentTime = oslab_get_internal_globaltime();
        
        
        driveState = driveInstructionButtons();
        
        
        driveFunction(currentTime, driveState); //Fukntion ansvarig f�r servo kontroll
        
        
        //N�r bot �r stillast�ende s� aktivera larmsensorer efter fyra sekunder
        
        
        
    }
}

/*
 * Mainfunktion - initialiserar alla variabler f�r f�rsta g�ngen, ger sedan �ver
 * kontrollen till mainLoop. 
 */
int main( void )
{   
    initialize();
    
    mainLoop();
}







