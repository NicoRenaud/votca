/*
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE pdbreader_test
#include <boost/test/unit_test.hpp>

#include <cmath>
#include <fstream>
#include <string>
#include <votca/csg/bead.h>
#include <votca/csg/topologyreader.h>
#include <votca/tools/elements.h>

using namespace std;
using namespace votca::csg;

// used for rounding doubles so we can compare them
double round_(double v, int p) {
  v *= pow(10, p);
  v = round(v);
  v /= pow(10, p);
  return v;
}

BOOST_AUTO_TEST_SUITE(pdbreader_test)

BOOST_AUTO_TEST_CASE(test_topologyreader) {

  //////////////////////////////////////////////////////////////////////////
  // Create pdb test file
  //////////////////////////////////////////////////////////////////////////
  ofstream outfile("Molecule1.pdb");
  outfile << "COMPND    UNNAMED" << endl;
  outfile << "AUTHOR    GENERATED BY OPEN BABEL 2.4.1" << endl;
  outfile << "HETATM    1  C   UNL     1      -5.249   1.055  -0.000  1.00  "
             "0.00           C"
          << endl;
  outfile << "HETATM    2  H   UNL     1      -6.202   1.521  -0.141  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM    3  H   UNL     1      -5.390   0.026   0.255  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM    4  H   UNL     1      -4.682   1.124  -0.904  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM    5  H   UNL     1      -4.724   1.550   0.790  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM    6  C   UNL     2      -2.248   1.671  -0.000  1.00  "
             "0.00           C"
          << endl;
  outfile << "HETATM    7  H   UNL     2      -1.518   2.451   0.051  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM    8  H   UNL     2      -3.153   1.999   0.467  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM    9  H   UNL     2      -2.442   1.430  -1.024  1.00  "
             "0.00           H"
          << endl;
  outfile << "HETATM   10  H   UNL     2      -1.880   0.804   0.507  1.00  "
             "0.00           H"
          << endl;
  outfile << "CONECT    1    2    3    4    5" << endl;
  outfile << "CONECT    2    1" << endl;
  outfile << "CONECT    3    1" << endl;
  outfile << "CONECT    4    1" << endl;
  outfile << "CONECT    5    1" << endl;
  outfile << "CONECT    6    7    8    9   10" << endl;
  outfile << "CONECT    7    6" << endl;
  outfile << "CONECT    8    6" << endl;
  outfile << "CONECT    9    6" << endl;
  outfile << "CONECT   10    6" << endl;
  outfile << "MASTER        0    0    0    0    0    0    0    0   10    0   "
             "10    0"
          << endl;
  outfile << "END" << endl;
  outfile.close();
  //////////////////////////////////////////////////////////////////////////

  Elements ele;

  Topology top;
  TopologyReader::RegisterPlugins();
  TopologyReader* reader;
  string          str = "Molecule1.pdb";
  reader              = TopReaderFactory().Create(str);
  reader->ReadTopology(str, top);
  BOOST_CHECK_EQUAL(reader != NULL, true);
  BOOST_CHECK_EQUAL(top.BeadCount(), 10);

  vector<int> resnr = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
  vector<string> bd_name = { "C", "H", "H", "H", "H", "C", "H", "H", "H", "H"};
  vector<double> x = { -0.5249, -0.6202,-0.539, -0.4682,-0.4724,-0.2248,-0.1518,-0.3153,-0.2442,-0.1880 };
  vector<double> y = { 0.1055 ,  0.1521, 0.0026, 0.1124, 0.1550, 0.1671, 0.2451, 0.1999, 0.1430, 0.0804 };
  vector<double> z = { -0.000 , -0.0141, 0.0255,-0.0904, 0.079 ,-0.000 , 0.0051, 0.0467,-0.1024, 0.0507 };
  Bead* bd;
  vec v;
  for(int i=0;i<10;i++){
    bd = top.getBead(i);
    BOOST_CHECK_EQUAL(bd->getId(),i);
    BOOST_CHECK_EQUAL(bd->getResnr(),resnr.at(i));
    BOOST_CHECK_EQUAL(bd->getName(),bd_name.at(i));
  //BOOST_CHECK_EQUAL(bd->getM(),ele.getMass(bd->getName()));
    v = bd->getPos();
    BOOST_CHECK_EQUAL(bd->getQ(), 0);
    BOOST_CHECK_EQUAL(round_(v.getX(), 3), round_(x.at(i), 3));
    BOOST_CHECK_EQUAL(round_(v.getY(), 3), round_(y.at(i), 3));
    BOOST_CHECK_EQUAL(round_(v.getZ(), 3), round_(z.at(i), 3));
  }
}

BOOST_AUTO_TEST_SUITE_END()
