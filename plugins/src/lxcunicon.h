/*
 ***
 *  Subject : Unicon LXC Container Library
 *  Author  : Jafar Al-Gharaibeh
 *  Date    : February 3, 2017
 *  Modified:
 ***
 *  This file is in the public domain.
 ***
 *  Describtion: provides basic function for creating and
 *               managing LXC containers 
 ***
 * compile:  gcc -std=c99 -shared -fpic -o luxicon.so luxicon.c -llxc
 ***
*/

word
lxcinit(int argc, descriptor *argv);

word
exist(int argc, descriptor *argv);

word
create(int argc, descriptor *argv);

word
start(int argc, descriptor *argv);

word
state(int argc, descriptor *argv);

word
stop(int argc, descriptor *argv);

word
shutdown(int argc, descriptor *argv);

word
destroy(int argc, descriptor *argv);

word
freeze(int argc, descriptor *argv);

word
unfreeze(int argc, descriptor *argv);

word
reboot(int argc, descriptor *argv);

word
attach(int argc, descriptor *argv);
