#!/bin/bash

Z_start="^[0-9][0-9]*: Z [0-9][0-9]*: started$"
Z_start_n=0

Z_in="^[0-9][0-9]*: Z [0-9][0-9]*: entering office for a service [1-3]$"
Z_in_n=0

Z_called="^[0-9][0-9]*: Z [0-9][0-9]*: called by office worker$"
Z_called_n=0

Z_home="^[0-9][0-9]*: Z [0-9][0-9]*: going home$"
Z_home_n=0

U_start="^[0-9][0-9]*: U [0-9][0-9]*: started$"
U_start_n=0

U_home="^[0-9][0-9]*: U [0-9][0-9]*: going home$"
U_home_n=0

U_break="^[0-9][0-9]*: U [0-9][0-9]*: taking break$"
U_break_n=0

U_breakF="^[0-9][0-9]*: U [0-9][0-9]*: break finished$"
U_breakF_n=0

U_serving="^[0-9][0-9]*: U [0-9][0-9]*: serving a service of type [1-3]$"
U_serving_n=0

U_servingF="^[0-9][0-9]*: U [0-9][0-9]*: service finished$"
U_servingF_n=0

Closing="^[0-9][0-9]*: closing$"
Closing_n=0

while read line
do
  if echo $line | grep "$Z_start" >/dev/null; then
  	Z_start_n=1
  elif echo $line | grep "$Z_in" >/dev/null; then
  	Z_in_n=1
  elif echo $line | grep "$Z_called" >/dev/null; then
  	Z_called_n=1
  elif echo $line | grep "$Z_home" >/dev/null; then
  	Z_home_n=1
  elif echo $line | grep "$U_start" >/dev/null; then
  	U_start_n=1
  elif echo $line | grep "$U_home" >/dev/null; then
  	U_home_n=1
  elif echo $line | grep "$U_break" >/dev/null; then
  	U_break_n=1
  elif echo $line | grep "$U_breakF" >/dev/null; then
  	U_breakF_n=1
  elif echo $line | grep "$U_serving" >/dev/null; then
  	U_serving_n=1
  elif echo $line | grep "$U_servingF" >/dev/null; then
  	U_servingF_n=1
  elif echo $line | grep "$Closing" >/dev/null; then
  	Closing_n=1
  else
  	echo "Line format error:" $line
  fi
done

if [ ! X$Z_start_n = "X1" ]; then 
	echo "WARNING: no Z started"
fi
if [ ! X$Z_in_n = "X1" ]; then 
	echo "WARNING: no Z entering office"
fi
if [ ! X$Z_called_n = "X1" ]; then 
	echo "WARNING: no Z called by office worker"
fi
if [ ! X$Z_home_n = "X1" ]; then 
	echo "WARNING: no Z going home"
fi
if [ ! X$U_start_n = "X1" ]; then 
	echo "WARNING: no U started"
fi
if [ ! X$U_break_n = "X1" ]; then 
	echo "WARNING: no U taking break"
fi
if [ ! X$U_breakF_n = "X1" ]; then 
	echo "WARNING: no U finishing break"
fi
if [ ! X$U_serving_n = "X1" ]; then 
	echo "WARNING: no U serving a service"
fi
if [ ! X$U_servingF_n = "X1" ]; then 
	echo "WARNING: no U finished a service"
fi
if [ ! X$Closing_n = "X1" ]; then 
	echo "WARNING: no closing"
fi

