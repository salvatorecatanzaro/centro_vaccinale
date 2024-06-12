#!/bin/bash

gcc -c -o utils.o utils.c
gcc -o client_a client_a.c
gcc -o centro_vaccinale centro_vaccinale.c utils.o
gcc -pthread -o server_v server_v.c utils.o
gcc -o server_g server_g.c utils.o
gcc -o client_s client_s.c
gcc -o client_t client_t.c

