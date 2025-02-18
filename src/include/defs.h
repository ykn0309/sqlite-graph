/*
 * @Author: Kainan Yang ykn0309@whu.edu.cn
 * @Date: 2024-11-29 17:06:08
 * @LastEditors: Kainan Yang ykn0309@whu.edu.cn
 * @LastEditTime: 2024-12-16 10:27:15
 * @FilePath: /sqlite-graph/src/defs.h
 * @Description: 
 * 
 */

#ifndef DEFS_H
#define DEFS_H

#define GRAPH_SUCCESS 0
#define GRAPH_FAILED -1
#define UPDATE_LABEL 0
#define UPDATE_ATTRIBUTE 1
#define UPDATE_LABEL_ATTRIBUTE 2
#define UPDATE_FROM 3
#define UPDATE_TO 4
#define UPDATE_FROM_TO 5
#define NODE 0
#define EDGE 1
#define NOCONSTRAIN 0 // no constrain
#define DEFINITE 1 // definite label
#define ATTRIBUTE 2 // attribute constrain
#define VARIABLE 3
#define LABEL 4

#endif