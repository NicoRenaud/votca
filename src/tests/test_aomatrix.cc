/*
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
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

#define BOOST_TEST_MODULE gnode_test
#include <boost/test/unit_test.hpp>
#include <votca/xtp/orbitals.h>
#include <votca/xtp/aomatrix.h>

using namespace votca::xtp;

BOOST_AUTO_TEST_SUITE(aomatrix_test)

BOOST_AUTO_TEST_CASE(aomatrices_test) {
  
  ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << endl;
  xyzfile << " methane" << endl;
  xyzfile << " C            .000000     .000000     .000000" << endl;
  xyzfile << " H            .629118     .629118     .629118" << endl;
  xyzfile << " H           q-.629118    -.629118     .629118" << endl;
  xyzfile << " H            .629118    -.629118    -.629118" << endl;
  xyzfile << " H           -.629118     .629118    -.629118" << endl;
  xyzfile.close();

  ofstream basisfile("3-21G.xml");
  basisfile <<"<basis name=\"3-21G\">" << endl;
  basisfile << "  <element name=\"H\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "  <element name=\"C\">" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>" << endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>" << endl;
  basisfile << "      </constant>" << endl;
  basisfile << "    </shell>" << endl;
  basisfile << "  </element>" << endl;
  basisfile << "</basis>" << endl;
  basisfile.close();
  
  Orbitals orbitals;
  orbitals.LoadFromXYZ("molecule.xyz");
  BasisSet basis;
  basis.LoadBasisSet("3-21G.xml");
  AOBasis aobasis;
  aobasis.AOBasisFill(&basis,orbitals.QMAtoms());
  AOOverlap overlap;
  overlap.Fill(aobasis);
  cout<<overlap.Matrix()<<endl<<endl;
  Eigen::MatrixXd overlap_ref=Eigen::MatrixXd::Zero(17,17);
 
  overlap_ref<<
1,0.191448,0,0,0,0.180314,0,0,0,0.0189724,0.0808612,0.0189724,0.0808612,0.0189724,0.0808612,0.0189724,0.0808612,
0.191448,1.00001,0,0,0,0.761361,0,0,0,0.194748,0.401447,0.194748,0.401447,0.194748,0.401447,0.194748,0.401447,
0,0,1,0,0,0,0.528959,0,0,0.169584,0.135615,0.169584,0.135615,-0.169584,-0.135615,-0.169584,-0.135615,
0,0,0,1,0,0,0,0.528959,0,0.169584,0.135615,-0.169584,-0.135615,-0.169584,-0.135615,0.169584,0.135615,
0,0,0,0,1,0,0,0,0.528959,0.169584,0.135615,-0.169584,-0.135615,0.169584,0.135615,-0.169584,-0.135615,
0.180314,0.761361,0,0,0,1,0,0,0,0.338796,0.668849,0.338796,0.668849,0.338796,0.668849,0.338796,0.668849,
0,0,0.528959,0,0,0,1,0,0,0.290649,0.340149,0.290649,0.340149,-0.290649,-0.340149,-0.290649,-0.340149,
0,0,0,0.528959,0,0,0,1,0,0.290649,0.340149,-0.290649,-0.340149,-0.290649,-0.340149,0.290649,0.340149,
0,0,0,0,0.528959,0,0,0,1,0.290649,0.340149,-0.290649,-0.340149,0.290649,0.340149,-0.290649,-0.340149,
0.0189724,0.194748,0.169584,0.169584,0.169584,0.338796,0.290649,0.290649,0.290649,1,0.645899,0.00778321,0.116994,0.00778321,0.116994,0.00778321,0.116994,
0.0808612,0.401447,0.135615,0.135615,0.135615,0.668849,0.340149,0.340149,0.340149,0.645899,1,0.116994,0.354983,0.116994,0.354983,0.116994,0.354983,
0.0189724,0.194748,0.169584,-0.169584,-0.169584,0.338796,0.290649,-0.290649,-0.290649,0.00778321,0.116994,1,0.645899,0.00778321,0.116994,0.00778321,0.116994,
0.0808612,0.401447,0.135615,-0.135615,-0.135615,0.668849,0.340149,-0.340149,-0.340149,0.116994,0.354983,0.645899,1,0.116994,0.354983,0.116994,0.354983,
0.0189724,0.194748,-0.169584,-0.169584,0.169584,0.338796,-0.290649,-0.290649,0.290649,0.00778321,0.116994,0.00778321,0.116994,1,0.645899,0.00778321,0.116994,
0.0808612,0.401447,-0.135615,-0.135615,0.135615,0.668849,-0.340149,-0.340149,0.340149,0.116994,0.354983,0.116994,0.354983,0.645899,1,0.116994,0.354983,
0.0189724,0.194748,-0.169584,0.169584,-0.169584,0.338796,-0.290649,0.290649,-0.290649,0.00778321,0.116994,0.00778321,0.116994,0.00778321,0.116994,1,0.645899,
0.0808612,0.401447,-0.135615,0.135615,-0.135615,0.668849,-0.340149,0.340149,-0.340149,0.116994,0.354983,0.116994,0.354983,0.116994,0.354983,0.645899,1;
cout<<overlap_ref<<endl;
cout<<(overlap.Matrix() - overlap_ref).norm() <<endl;
bool check_overlap = overlap.Matrix().isApprox(overlap_ref,0.0001);
BOOST_CHECK_EQUAL(check_overlap, 1);

AOKinetic kinetic;
kinetic.Fill(aobasis);
Eigen::MatrixXd kinetic_ref=Eigen::MatrixXd::Zero(17,17);
kinetic_ref<<16.579,-1.43503,0,0,0,0.10275,0,0,0,-0.0439437,0.0214514,-0.0439437,0.0214514,-0.0439437,0.0214514,-0.0439437,0.0214514,
-1.43503,1.35738,0,0,0,0.346414,0,0,0,-0.0154143,0.103305,-0.0154143,0.103305,-0.0154143,0.103305,-0.0154143,0.103305,
0,0,2.58667,0,0,0,0.41751,0,0,0.0928809,0.0755731,0.0928809,0.0755731,-0.0928809,-0.0755731,-0.0928809,-0.0755731,
0,0,0,2.58667,0,0,0,0.41751,0,0.0928809,0.0755731,-0.0928809,-0.0755731,-0.0928809,-0.0755731,0.0928809,0.0755731,
0,0,0,0,2.58667,0,0,0,0.41751,0.0928809,0.0755731,-0.0928809,-0.0755731,0.0928809,0.0755731,-0.0928809,-0.0755731,
0.10275,0.346414,0,0,0,0.293786,0,0,0,0.0889197,0.139112,0.0889197,0.139112,0.0889197,0.139112,0.0889197,0.139112,
0,0,0.41751,0,0,0,0.489642,0,0,0.169257,0.135141,0.169257,0.135141,-0.169257,-0.135141,-0.169257,-0.135141,
0,0,0,0.41751,0,0,0,0.489642,0,0.169257,0.135141,-0.169257,-0.135141,-0.169257,-0.135141,0.169257,0.135141,
0,0,0,0,0.41751,0,0,0,0.489642,0.169257,0.135141,-0.169257,-0.135141,0.169257,0.135141,-0.169257,-0.135141,
-0.0439437,-0.0154143,0.0928809,0.0928809,0.0928809,0.0889197,0.169257,0.169257,0.169257,1.5494,0.293152,-0.0206172,-0.00736852,-0.0206172,-0.00736852,-0.0206172,-0.00736852,
0.0214514,0.103305,0.0755731,0.0755731,0.0755731,0.139112,0.135141,0.135141,0.135141,0.293152,0.274788,-0.00736852,0.0301943,-0.00736852,0.0301943,-0.00736852,0.0301943,
-0.0439437,-0.0154143,0.0928809,-0.0928809,-0.0928809,0.0889197,0.169257,-0.169257,-0.169257,-0.0206172,-0.00736852,1.5494,0.293152,-0.0206172,-0.00736852,-0.0206172,-0.00736852,
0.0214514,0.103305,0.0755731,-0.0755731,-0.0755731,0.139112,0.135141,-0.135141,-0.135141,-0.00736852,0.0301943,0.293152,0.274788,-0.00736852,0.0301943,-0.00736852,0.0301943,
-0.0439437,-0.0154143,-0.0928809,-0.0928809,0.0928809,0.0889197,-0.169257,-0.169257,0.169257,-0.0206172,-0.00736852,-0.0206172,-0.00736852,1.5494,0.293152,-0.0206172,-0.00736852,
0.0214514,0.103305,-0.0755731,-0.0755731,0.0755731,0.139112,-0.135141,-0.135141,0.135141,-0.00736852,0.0301943,-0.00736852,0.0301943,0.293152,0.274788,-0.00736852,0.0301943,
-0.0439437,-0.0154143,-0.0928809,0.0928809,-0.0928809,0.0889197,-0.169257,0.169257,-0.169257,-0.0206172,-0.00736852,-0.0206172,-0.00736852,-0.0206172,-0.00736852,1.5494,0.293152,
0.0214514,0.103305,-0.0755731,0.0755731,-0.0755731,0.139112,-0.135141,0.135141,-0.135141,-0.00736852,0.0301943,-0.00736852,0.0301943,-0.00736852,0.0301943,0.293152,0.274788;

bool check_kinetic = kinetic.Matrix().isApprox(kinetic_ref,0.00001);
BOOST_CHECK_EQUAL(check_kinetic, 1); 

AOESP esp;
esp.Fillnucpotential(aobasis,orbitals.QMAtoms());
Eigen::MatrixXd esp_ref=Eigen::MatrixXd::Zero(17,17);

esp_ref<<0.485633,0.0929737,-0.0112854,0.0112854,-0.0112854,0.0875665,-0.00167437,0.00167437,-0.00167437,0.00881413,0.0388865,0.00881413,0.0388865,0.00881413,0.0388865,0.010454,0.0404184,
0.0929737,0.485422,-0.0825499,0.0825499,-0.0825499,0.367955,-0.0505522,0.0505522,-0.0505522,0.0731397,0.178738,0.0731397,0.178738,0.0731397,0.178738,0.194659,0.240204,
-0.0112854,-0.0825499,0.485023,-0.0299598,0.0299598,-0.0781392,0.252799,-0.0256949,0.0256949,0.0563721,0.0217104,0.0563721,0.0217104,-0.0677234,-0.0882558,-0.197548,-0.133855,
0.0112854,0.0825499,-0.0299598,0.485023,-0.0299598,0.0781392,-0.0256949,0.252799,-0.0256949,0.0677234,0.0882558,-0.0563721,-0.0217104,-0.0563721,-0.0217104,0.197548,0.133855,
-0.0112854,-0.0825499,0.0299598,-0.0299598,0.485023,-0.0781392,0.0256949,-0.0256949,0.252799,0.0563721,0.0217104,-0.0677234,-0.0882558,0.0563721,0.0217104,-0.197548,-0.133855,
0.0875665,0.367955,-0.0781392,0.0781392,-0.0781392,0.452433,-0.100822,0.100822,-0.100822,0.110741,0.253817,0.110741,0.253817,0.110741,0.253817,0.391022,0.405943,
-0.00167437,-0.0505522,0.252799,-0.0256949,0.0256949,-0.100822,0.407715,-0.0510951,0.0510951,0.084227,0.0617612,0.084227,0.0617612,-0.0960714,-0.152532,-0.362371,-0.271961,
0.00167437,0.0505522,-0.0256949,0.252799,-0.0256949,0.100822,-0.0510951,0.407715,-0.0510951,0.0960714,0.152532,-0.084227,-0.0617612,-0.084227,-0.0617612,0.362371,0.271961,
-0.00167437,-0.0505522,0.0256949,-0.0256949,0.252799,-0.100822,0.0510951,-0.0510951,0.407715,0.084227,0.0617612,-0.0960714,-0.152532,0.084227,0.0617612,-0.362371,-0.271961,
0.00881413,0.0731397,0.0563721,0.0677234,0.0563721,0.110741,0.084227,0.0960714,0.084227,0.297388,0.192083,0.0026714,0.0376198,0.0026714,0.0376198,0.00464633,0.042269,
0.0388865,0.178738,0.0217104,0.0882558,0.0217104,0.253817,0.0617612,0.152532,0.0617612,0.192083,0.2962,0.0376198,0.120354,0.0376198,0.120354,0.124795,0.179447,
0.00881413,0.0731397,0.0563721,-0.0563721,-0.0677234,0.110741,0.084227,-0.084227,-0.0960714,0.0026714,0.0376198,0.297388,0.192083,0.0026714,0.0376198,0.00464633,0.042269,
0.0388865,0.178738,0.0217104,-0.0217104,-0.0882558,0.253817,0.0617612,-0.0617612,-0.152532,0.0376198,0.120354,0.192083,0.2962,0.0376198,0.120354,0.124795,0.179447,
0.00881413,0.0731397,-0.0677234,-0.0563721,0.0563721,0.110741,-0.0960714,-0.084227,0.084227,0.0026714,0.0376198,0.0026714,0.0376198,0.297388,0.192083,0.00464633,0.042269,
0.0388865,0.178738,-0.0882558,-0.0217104,0.0217104,0.253817,-0.152532,-0.0617612,0.0617612,0.0376198,0.120354,0.0376198,0.120354,0.192083,0.2962,0.124795,0.179447,
0.010454,0.194659,-0.197548,0.197548,-0.197548,0.391022,-0.362371,0.362371,-0.362371,0.00464633,0.124795,0.00464633,0.124795,0.00464633,0.124795,1.72092,0.782663,
0.0404184,0.240204,-0.133855,0.133855,-0.133855,0.405943,-0.271961,0.271961,-0.271961,0.042269,0.179447,0.042269,0.179447,0.042269,0.179447,0.782663,0.683004;

bool check_esp = esp.Matrix().isApprox(esp_ref,0.00001);
BOOST_CHECK_EQUAL(check_esp, 1);

ofstream ecpfile("ecp.xml");
ecpfile << "<pseudopotential name=\"ECP_STUTTGART\">" << endl;
ecpfile << "  <element name=\"C\" lmax=\"3\" ncore=\"2\">" << endl;
ecpfile << "    <shell type=\"F\"><constant power=\"2\" decay=\"1.0\" contraction=\"0.0\"></constant></shell>" << endl;
ecpfile << "    <shell type=\"S\"><constant power=\"2\" decay=\"6.40105200\" contraction=\"33.12163800\"></constant></shell>" << endl;
ecpfile << "    <shell type=\"P\"><constant power=\"2\" decay=\"7.30774700\" contraction=\"-1.98625700\"></constant></shell>" << endl;
ecpfile << "    <shell type=\"D\"><constant power=\"2\" decay=\"5.96179600\" contraction=\"-9.45431800\"></constant></shell>" << endl;
ecpfile << "  </element>" << endl;
ecpfile << "</pseudopotential>" << endl;

BasisSet ecps;
ecps.LoadPseudopotentialSet("ecp.xml");
AOBasis ecpbasis;
ecpbasis.ECPFill(&ecps,orbitals.QMAtoms());

AOECP ecp;
ecp.Fill(aobasis,vec(0.0),&ecpbasis);
Eigen::MatrixXd ecp_ref= Eigen::MatrixXd::Zero(17,17);
ecp_ref<<21.6188,1.34835,0,0,0,2.29744,0,0,0,0.209711,1.01592,0.209711,1.01592,0.209711,1.01592,0.209711,1.01592,
1.34835,0.702249,0,0,0,0.4993,0,0,0,0.0564639,0.225665,0.0564639,0.225665,0.0564639,0.225665,0.0564639,0.225665,
0,0,-0.0737545,0,0,0,-0.00882987,0,0,-0.00178626,-0.00193605,-0.00178626,-0.00193605,0.00178626,0.00193605,0.00178626,0.00193605,
0,0,0,-0.0737545,0,0,0,-0.00882987,0,-0.00178626,-0.00193605,0.00178626,0.00193605,0.00178626,0.00193605,-0.00178626,-0.00193605,
0,0,0,0,-0.0737545,0,0,0,-0.00882987,-0.00178626,-0.00193605,0.00178626,0.00193605,-0.00178626,-0.00193605,0.00178626,0.00193605,
2.29744,0.4993,0,0,0,0.458665,0,0,0,0.0477375,0.20545,0.0477375,0.20545,0.0477375,0.20545,0.0477375,0.20545,
0,0,-0.00882987,0,0,0,-0.0011596,0,0,-0.000240513,-0.000255319,-0.000240513,-0.000255319,0.000240513,0.000255319,0.000240513,0.000255319,
0,0,0,-0.00882987,0,0,0,-0.0011596,0,-0.000240513,-0.000255319,0.000240513,0.000255319,0.000240513,0.000255319,-0.000240513,-0.000255319,
0,0,0,0,-0.00882987,0,0,0,-0.0011596,-0.000240513,-0.000255319,0.000240513,0.000255319,-0.000240513,-0.000255319,0.000240513,0.000255319,
0.209711,0.0564639,-0.00178626,-0.00178626,-0.00178626,0.0477375,-0.000240513,-0.000240513,-0.000240513,0.00468574,0.0212243,0.0052935,0.0215396,0.0052935,0.0215396,0.0052935,0.0215396,
1.01592,0.225665,-0.00193605,-0.00193605,-0.00193605,0.20545,-0.000255319,-0.000255319,-0.000255319,0.0212243,0.0918741,0.0215396,0.0921252,0.0215396,0.0921252,0.0215396,0.0921252,
0.209711,0.0564639,-0.00178626,0.00178626,0.00178626,0.0477375,-0.000240513,0.000240513,0.000240513,0.0052935,0.0215396,0.00468574,0.0212243,0.0052935,0.0215396,0.0052935,0.0215396,
1.01592,0.225665,-0.00193605,0.00193605,0.00193605,0.20545,-0.000255319,0.000255319,0.000255319,0.0215396,0.0921252,0.0212243,0.0918741,0.0215396,0.0921252,0.0215396,0.0921252,
0.209711,0.0564639,0.00178626,0.00178626,-0.00178626,0.0477375,0.000240513,0.000240513,-0.000240513,0.0052935,0.0215396,0.0052935,0.0215396,0.00468574,0.0212243,0.0052935,0.0215396,
1.01592,0.225665,0.00193605,0.00193605,-0.00193605,0.20545,0.000255319,0.000255319,-0.000255319,0.0215396,0.0921252,0.0215396,0.0921252,0.0212243,0.0918741,0.0215396,0.0921252,
0.209711,0.0564639,0.00178626,-0.00178626,0.00178626,0.0477375,0.000240513,-0.000240513,0.000240513,0.0052935,0.0215396,0.0052935,0.0215396,0.0052935,0.0215396,0.00468574,0.0212243,
1.01592,0.225665,0.00193605,-0.00193605,0.00193605,0.20545,0.000255319,-0.000255319,0.000255319,0.0215396,0.0921252,0.0215396,0.0921252,0.0215396,0.0921252,0.0212243,0.0918741;

bool check_ecp = ecp.Matrix().isApprox(ecp_ref,0.00001);
BOOST_CHECK_EQUAL(check_ecp, 1);

}

BOOST_AUTO_TEST_SUITE_END()
