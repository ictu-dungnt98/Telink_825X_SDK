/********************************************************************************************************
 * @file     string.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 public@telink-semi.com;
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/

#pragma once

#include <stddef.h>

void *  memmove(void * dest, const void * src, size_t n);
void *  memset(void * d, int c, size_t  n);
void *  memcpy(void * des_ptr, const void * src_ptr, size_t);

// do not return void*,  otherwise, we must use a variable to store the dest porinter, that is not performance 
void   	memcpy4(void * dest, const void * src, size_t);
void *	memchr(const void *_s, int _c, size_t _n);
int		memcmp(const void *_s1, const void *_s2, size_t _n);

char *	strcat(char *_s1, const char *_s2);
char *	strchr(const char *_s, int _c);
int		strcmp(const char *_s1, const char *_s2);
int     strxcmp(const char* firstString, const char* secondString);
int		strcoll(const char *_s1, const char *_s2);
char *	strcpy(char *_s1, const char *_s2);
size_t	strcspn(const char *_s1, const char *_s2);
char *	strerror(int _errcode);
size_t	strlen(const char *_s);
char *	strncat(char *_s1, const char *_s2, size_t _n);
int		strncmp(const char *_s1, const char *_s2, size_t _n);
char *	strncpy(char *_s1, const char *_s2, size_t _n);
char *	strpbrk(const char *_s1, const char *_s2);
char *	strrchr(const char *_s, int _c);
size_t	strspn(const char *_s1, const char *_s2);
char *	strstr(const char *_s1, const char *_s2);
char *	strxstr(const char *_s1, const char *_s2);
char *	strtok(char *_s1, const char *_s2);
size_t	strxfrm(char *_s1, const char *_s2, size_t _n);
char * 	strchr (register const char *s, int c);
void * 	memchr (register const void * src_void, int c, size_t length);
int ismemzero4(void *data, size_t len);
int ismemf4(void *data, size_t len);
void * memset4(void * dest, int val, size_t len);
void zeromem4(void *data, size_t len);

