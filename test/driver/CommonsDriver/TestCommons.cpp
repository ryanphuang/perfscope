/**
 *  @file          test/driver/CommonsDriver/CommonsTest.cpp
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
#include <string>

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"

#include "commons/handy.h"
#include "commons/CallSiteFinder.h"
#include "commons/LLVMHelper.h"

using namespace std;
using namespace llvm;

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

inline static void end_test(const char *name, int total, int failed)
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


typedef struct pendswith_T {
  const char * str;
  const char * ending;
} pendswith_T;

static const pendswith_T pendswith_test[] = {
  {"innobase/fil/fil0fil.c", "/fil0fil.c"},
  {"foo/bar/ba.c", "a.c"},
  {"foo/bar/ba.c", "foo/bar/ba.c"},
  {"foo/bar/ba.c", "/barr/ba.c"},
  {"foo/bar/ba.c", "bar/ba.c"},
  {0, 0}
};

static const int pendswith_expect[] = {
  1,
  -1,
  1,
  -1,
  1,
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

void test_pendswith()
{
  int total = 0, failed = 0;
  bool fail = false;
  begin_test("pendswith");
  const pendswith_T *t = pendswith_test;
  const int *e = pendswith_expect;
  int result, expect;
  while (t->str && *e) {
    result = pendswith(t->str, t->ending);
    expect = (*e == 1 ? 1 : 0);
    if((fail = (result != expect))) {
      one_test(total, failed, fail, expect == 1 ? "true" : "false", 
        result ? "true" : "false");
    }
    else
      one_test(total, failed, fail);
    t++;
    e++;
  }
  end_test("pendswith", total, failed);


}

void test_CallGraph(Module *module, const char * fname)
{
  if (module != NULL) {
    Function* func = module->getFunction(fname);
    if (func == NULL) {
      cout << "Cannot find function '" << fname << "'" << endl;
      return;
    }
    CallSiteFinder csf(func);
    CallSiteFinder::cs_iterator ci = csf.begin(), ce = csf.end();
    if(ci == ce) {
      cout << "No function";
    }
    else {
      cout << "The following functions";
    }
    const char *name = func->getName().data();
    cout << " called " << cpp_demangle(name) << endl;
    for (; ci != ce; ++ci) {
      name = (ci->first)->getName().data();
      cout << cpp_demangle(name) << endl;
    }
  }
}

void test_cppdemangle()
{
  begin_test("cppdemangle");
  const char * name = "_ZL13store_triggerP3THDP8st_tableP19st_mysql_"
        "lex_stringS4_S4_14trg_event_type20trg_action_time_typeS4_mS4_S4_S4_S4_";
  // test if gcc bug 42230 would be hit
  int failed = 0;
  if (strcmp(cpp_demangle(name), "store_trigger(THD*, st_table*, st_mysql_lex_string*"
        ", st_mysql_lex_string*, st_mysql_lex_string*, trg_event_type, trg_action_time"
        "_type, st_mysql_lex_string*, unsigned long, st_mysql_lex_string*, st_mysql_lex"
        "_string*, st_mysql_lex_string*, st_mysql_lex_string*)") != 0)
    failed++;
  name = "_ZN4llvm12MapGraphBaseIPNS_11InstructionEED1Ev";
  if (strcmp(cpp_demangle(name), "llvm::MapGraphBase<llvm::Instruction*>::~MapGraphBase()") != 0)
    failed++;
  end_test("cppdemangle", 2, 0);
}

int main()
{
  test_cppdemangle();
  test_src2obj();
  test_canonpath();
  test_pendswith();
  test_stripname();
  return 0;
}

