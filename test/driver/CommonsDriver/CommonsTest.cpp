/**
 *  @file          /home/ryan/project/perfscope/test/driver/CommonsDriver/CommonsTest.cpp
 *
 *  @version       1.0
 *  @created       02/04/2013 05:57:23 PM
 *  @revision      $Id$
 *
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 *  
 *  Copyright (c) 2013, Ryan Huang
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *  http://www.apache.org/licenses/LICENSE-2.0
 *     
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @section       DESCRIPTION
 *  
 *  Test driver for the commons library
 *
 */

#include <iostream>
#include <map>

#include "commons/handy.h"

using namespace std;

inline static void begin_test(const char * name)
{
  cout << "Testing " << name << "..." << endl;
}

inline static void one_test(int & total, int & failed, bool fail, string expect = "", 
    string actual = "")
{
  cout << "Test case #" << total << " ";
  if (fail) {
    cout << "failed";
    if (!expect.empty())
      cout << ": expected(" << expect << "), got(" << actual << ")";
    cout << "." << endl;
  }
  else
    cout << "succeeded." << endl;
  total++;
  failed += fail;
}

inline static void end_test(const char *name, int & total, int & failed)
{
  cout << "Testing " << name << " finished: total " << total << ", " << failed << " failed." <<  endl;
}


typedef struct stripname_T {
  const char * name;
  int lstrips;
} stripname_T;

static const stripname_T stripname_test [] = {
  {"MYSQLPlus//MYSQLPlusTest/MYSQLPlusTest.cpp", -1},
  {"MYSQLPlus//MYSQLPlusTest/MYSQLPlusTest.cpp", 1},
  {"a//b/c///", -1},
  {"/home/ryan/Projects/llvm-exp/mysql-5.0.15/sql/./sql_string.h", 6},
  {0, 0}
};

static const char * stripname_expect [] = {
  "MYSQLPlusTest.cpp",
  "MYSQLPlusTest/MYSQLPlusTest.cpp",
  "",
  "sql/sql_string.h",
  0
};

static const char * canonpath_test[] = {
  "a/./b//c",
  "/a/./b//c/./d/",
  "home/ryan/./Documents//../Projects/",
  "home/../root",
  0
};

static const char * canonpath_expect[] = {
  "a/b/c",
  "/a/b/c/d",
  "home/ryan/Projects",
  "root",
  0
};

static const char * src2obj_test[] = {
  "storage/innobase/fil/fil0fil.c",
  "client/mysql_plugin.c",
  "include/m_ctype.h",
  0
};

static const char * src2obj_expect[] = {
  "storage/innobase/fil/fil0fil.o",
  "client/mysql_plugin.o",
  0
};

void test_canonpath()
{
  int total = 0, failed = 0;
  bool fail = false;
  begin_test("canonpath");
  const char **t = canonpath_test;
  const char **e = canonpath_expect;
  const char *result, *expect;
  while (*t && *e) {
    result = canonpath(*t, NULL);
    expect = *e;
    if((fail = (strcmp(result, expect) != 0))) {
      one_test(total, failed, fail, expect, result);
    }
    else
      one_test(total, failed, fail);
    t++;
    e++;
  }
  end_test("canonpath", total, failed);
}

void test_stripname()
{
  int total = 0, failed = 0;
  bool fail = false;
  begin_test("stripname");
  const stripname_T *t = stripname_test;
  const char **e = stripname_expect;
  const char *result, *expect;
  while (t->name && *e) {
    result = stripname(t->name, t->lstrips);
    expect = *e;
    if((fail = (strcmp(result, expect) != 0))) {
      one_test(total, failed, fail, expect, result);
    }
    else
      one_test(total, failed, fail);
    t++;
    e++;
  }
  end_test("stripname", total, failed);
}

static int objlen = MAX_PATH;
static char objname[MAX_PATH];

void test_src2obj()
{
  int total = 0, failed = 0;
  bool fail = false;
  begin_test("src2obj");
  const char **t = src2obj_test;
  const char **e = src2obj_expect;
  const char *result, *expect;
  while (*t) {
    result = src2obj(*t, objname, &objlen);
    expect = *e;
    fail = (expect == NULL ? (result != NULL) : 
          (strcmp(result, expect) != 0)); 
    if (fail) {
      if (expect == NULL)
        expect = "NULL"; 
      one_test(total, failed, fail, expect, result);
    }
    else
      one_test(total, failed, fail);
    t++;
    if (expect != NULL)
      e++;
  }
  end_test("src2obj", total, failed);
}

int main()
{

  test_src2obj();
  test_canonpath();
  test_stripname();
  return 0;
}

