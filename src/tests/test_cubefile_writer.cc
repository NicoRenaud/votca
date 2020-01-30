/*
 * Copyright 2009-2019 The VOTCA Development Team (http://www.votca.org)
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

#define BOOST_TEST_MODULE cubefilewriter_test
#include <boost/test/unit_test.hpp>
#include <votca/tools/tokenizer.h>
#include <votca/xtp/cubefile_writer.h>

using namespace votca::xtp;

BOOST_AUTO_TEST_SUITE(cubefilewriter_test)

Eigen::VectorXd Readcubefile(const std::string& filename) {

  std::ifstream in1;
  in1.open(filename, std::ios::in);

  std::string result = "";
  std::string s;
  getline(in1, s);
  getline(in1, s);
  getline(in1, s);
  std::vector<double> cube_values;
  do {
    votca::tools::Tokenizer tok(s, " ");
    std::vector<double> values;
    tok.ConvertToVector<double>(values);
    cube_values.insert(cube_values.end(), values.begin(), values.end());
  } while (getline(in1, s));
  return Eigen::Map<Eigen::VectorXd>(cube_values.data(), cube_values.size());
}

BOOST_AUTO_TEST_CASE(constructors_test) {

  std::ofstream xyzfile("molecule.xyz");
  xyzfile << " 5" << std::endl;
  xyzfile << " methane" << std::endl;
  xyzfile << " C            .000000     .000000     .000000" << std::endl;
  xyzfile << " H            .629118     .629118     .629118" << std::endl;
  xyzfile << " H           -.629118    -.629118     .629118" << std::endl;
  xyzfile << " H            .629118    -.629118    -.629118" << std::endl;
  xyzfile << " H           -.629118     .629118    -.629118" << std::endl;
  xyzfile.close();

  std::ofstream basisfile("3-21G.xml");
  basisfile << "<basis name=\"3-21G\">" << std::endl;
  basisfile << "  <element name=\"H\">" << std::endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << std::endl;
  basisfile << "      <constant decay=\"5.447178e+00\">" << std::endl;
  basisfile << "        <contractions factor=\"1.562850e-01\" type=\"S\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "      <constant decay=\"8.245470e-01\">" << std::endl;
  basisfile << "        <contractions factor=\"9.046910e-01\" type=\"S\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "    </shell>" << std::endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << std::endl;
  basisfile << "      <constant decay=\"1.831920e-01\">" << std::endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "    </shell>" << std::endl;
  basisfile << "  </element>" << std::endl;
  basisfile << "  <element name=\"C\">" << std::endl;
  basisfile << "    <shell scale=\"1.0\" type=\"S\">" << std::endl;
  basisfile << "      <constant decay=\"1.722560e+02\">" << std::endl;
  basisfile << "        <contractions factor=\"6.176690e-02\" type=\"S\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "      <constant decay=\"2.591090e+01\">" << std::endl;
  basisfile << "        <contractions factor=\"3.587940e-01\" type=\"S\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "      <constant decay=\"5.533350e+00\">" << std::endl;
  basisfile << "        <contractions factor=\"7.007130e-01\" type=\"S\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "    </shell>" << std::endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << std::endl;
  basisfile << "      <constant decay=\"3.664980e+00\">" << std::endl;
  basisfile << "        <contractions factor=\"-3.958970e-01\" type=\"S\"/>"
            << std::endl;
  basisfile << "        <contractions factor=\"2.364600e-01\" type=\"P\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "      <constant decay=\"7.705450e-01\">" << std::endl;
  basisfile << "        <contractions factor=\"1.215840e+00\" type=\"S\"/>"
            << std::endl;
  basisfile << "        <contractions factor=\"8.606190e-01\" type=\"P\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "    </shell>" << std::endl;
  basisfile << "    <shell scale=\"1.0\" type=\"SP\">" << std::endl;
  basisfile << "      <constant decay=\"1.958570e-01\">" << std::endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"S\"/>"
            << std::endl;
  basisfile << "        <contractions factor=\"1.000000e+00\" type=\"P\"/>"
            << std::endl;
  basisfile << "      </constant>" << std::endl;
  basisfile << "    </shell> " << std::endl;
  basisfile << "  </element>" << std::endl;
  basisfile << "</basis>" << std::endl;
  basisfile.close();

  Orbitals A;
  A.setDFTbasisName("3-21G.xml");
  A.QMAtoms().LoadFromFile("molecule.xyz");
  A.setBasisSetSize(17);
  A.setNumberOfAlphaElectrons(5);
  A.setNumberOfOccupiedLevels(5);
  A.MOs().eigenvalues() = Eigen::VectorXd::Zero(17);
  A.MOs().eigenvalues() << -19.8117, -6.22408, -6.14094, -6.14094, -6.14094,
      -3.72889, -3.72889, -3.72889, -3.64731, -3.09048, -3.09048, -3.09048,
      -2.63214, -2.08206, -2.08206, -2.08206, -2.03268;

  A.MOs().eigenvectors() = Eigen::MatrixXd::Zero(17, 17);
  A.MOs().eigenvectors() << -0.996559, -0.223082, 4.81443e-15, 2.21045e-15,
      -6.16146e-17, -3.16966e-16, 5.46703e-18, -1.09681e-15, -0.0301914,
      6.45993e-16, 1.05377e-16, 3.41154e-16, -0.102052, -5.79826e-16,
      9.38593e-16, -4.69346e-15, -0.111923, -0.0445146, 0.88316, -1.94941e-14,
      -8.67388e-15, -7.26679e-16, 1.16326e-14, -3.35886e-15, 2.37877e-14,
      0.866126, 3.2068e-15, 3.80914e-15, 3.24563e-15, -0.938329, -6.4404e-15,
      1.10811e-14, -5.5056e-14, -1.28767, 8.15798e-17, 2.30849e-14, 1.04169,
      0.117804, 0.0951759, 0.179467, 0.147031, 0.39183, -1.02927e-14, 0.32699,
      -0.223689, -0.130009, 1.0375e-15, -0.0940179, 0.126956, 0.0122904,
      1.41709e-15, 4.60157e-17, -7.1203e-15, 0.143338, -0.980459, -0.355251,
      0.41611, -0.10826, -0.149964, 2.41546e-16, 0.12214, -0.0512447, 0.39537,
      1.1054e-15, -0.0996828, -0.0636092, -0.105478, 5.10746e-15, -5.25872e-18,
      4.8424e-15, 0.0488925, 0.364515, -0.9863, 0.0447336, 0.417155, -0.177023,
      5.76117e-15, -0.228081, -0.348136, 0.0253377, -1.05286e-15, 0.079576,
      0.0703157, -0.117608, 5.31327e-15, 0.0472716, 0.235837, -3.58018e-15,
      -1.68354e-15, 2.3989e-15, -9.86879e-15, 4.52519e-15, -1.6106e-14,
      -0.599523, -1.31237e-14, -8.63443e-15, -8.61196e-15, 1.8333, 2.05275e-14,
      -3.9562e-14, 1.89874e-13, 4.24316, -2.74184e-16, -1.53939e-15, -0.162416,
      -0.0183675, -0.0148395, -0.151162, -0.123842, -0.330032, 1.10084e-15,
      -1.45092, 0.992556, 0.576875, -3.82954e-15, 0.604373, -0.816111,
      -0.0790061, -8.89474e-15, -2.24862e-16, 3.23655e-15, -0.0223487, 0.152869,
      0.0553894, -0.350483, 0.0911859, 0.126313, -5.48468e-15, -0.541961,
      0.227383, -1.75434, -3.89443e-15, 0.640788, 0.408897, 0.67804,
      -3.17156e-14, -2.81346e-17, -1.09423e-15, -0.00762313, -0.0568338,
      0.15378, -0.0376785, -0.351364, 0.149104, -4.94425e-15, 1.01204, 1.54475,
      -0.112429, 8.50653e-15, -0.511536, -0.452008, 0.756019, -3.3547e-14,
      -0.00106227, 0.0237672, 0.00345981, -0.00139675, -0.00349474, -0.597906,
      -0.425733, -0.0605479, -0.343823, 0.162103, -0.45692, 0.21318, -0.600309,
      0.310843, -0.36406, 0.574148, 0.0554949, -0.00928842, -0.0414346,
      0.0619999, -0.0250297, -0.0626259, 0.00227746, 0.00162164, 0.00023063,
      -0.0301047, 0.273177, -0.770004, 0.359253, 0.0095153, -0.8783, 1.02867,
      -1.62228, -1.24406, -0.00106227, 0.0237672, 0.00238182, 0.00205737,
      0.00402848, 0.262742, 0.151145, -0.671213, -0.343823, 0.317484, 0.12884,
      -0.40386, -0.600309, 0.201313, -0.327527, -0.641099, 0.0554949,
      -0.00928842, -0.0414346, 0.0426822, 0.0368682, 0.0721904, -0.0010008,
      -0.000575719, 0.00255669, -0.0301047, 0.535026, 0.217123, -0.680588,
      0.0095153, -0.568818, 0.925441, 1.81145, -1.24406, -0.00106227, 0.0237672,
      -0.00318563, 0.0034409, -0.00203628, 0.514364, -0.353326, 0.391148,
      -0.343823, -0.496623, -0.0536813, -0.176018, -0.600309, -0.744328,
      -0.01898, 0.0665156, 0.0554949, -0.00928842, -0.0414346, -0.0570866,
      0.0616609, -0.0364902, -0.00195924, 0.00134584, -0.0014899, -0.0301047,
      -0.836913, -0.0904642, -0.296627, 0.0095153, 2.10313, 0.0536287,
      -0.187943, -1.24406, -0.00106227, 0.0237672, -0.002656, -0.00410152,
      0.00150255, -0.1792, 0.627913, 0.340613, -0.343823, 0.0170366, 0.38176,
      0.366698, -0.600309, 0.232172, 0.710567, 0.000435528, 0.0554949,
      -0.00928842, -0.0414346, -0.0475955, -0.0734994, 0.0269257, 0.000682583,
      -0.00239176, -0.00129742, -0.0301047, 0.0287103, 0.643346, 0.617962,
      0.0095153, -0.656011, -2.00774, -0.0012306, -1.24406;

  A.setBSEindices(0, 16);
  A.setTDAApprox(true);
  A.setGWindices(0, 16);
  Eigen::MatrixXd spsi_ref = Eigen::MatrixXd::Zero(60, 3);
  spsi_ref << -0.00178316, -0.0558332, 0.0151767, 0.00568291, 0.0149417,
      0.0556358, 0.05758, -0.00320371, -0.00502105, 0.00379328, -0.00232255,
      -0.00817518, -0.00848959, -0.000618633, -0.00376334, -0.000395809,
      -0.00899117, 0.0023708, 7.13955e-08, 3.47498e-08, -1.08537e-08,
      0.000420413, 0.011896, -0.00320024, -0.00288487, 0.00320821, 0.0115465,
      0.0119767, 0.000355172, 0.00289343, -2.55565e-08, 1.91684e-08,
      3.01647e-09, 6.84051e-09, 2.79592e-10, -1.35767e-09, 0.00420618, 0.092451,
      -0.0239374, 0.0036276, 0.0113951, 0.0471896, 0.0465325, -0.00398807,
      -0.00484251, 0.0051614, -0.0031325, -0.0112447, -0.010968, -0.000896935,
      -0.00488789, 0.000951266, 0.0239709, -0.00630323, 0.000419006, 0.0201651,
      -0.00573095, -0.00118124, -0.0269275, 0.00700955, -0.00345217, 0.00356488,
      0.0134361, 0.013156, 9.58532e-05, 0.00315613, -2.58268e-05, -0.00124098,
      0.000352706, -1.80679e-06, -8.71959e-05, 2.47799e-05, -0.0150073,
      0.0114186, 0.0443012, 0.0180327, -0.026287, 0.0734351, -0.0643994,
      0.0257628, 0.0132084, -0.0123339, 0.0092297, -0.0148779, 0.0122493,
      -0.00246837, -0.0125735, -0.00375172, 0.00294872, 0.0112899, 0.00648758,
      -0.0055755, -0.0191436, 0.00433063, -0.00332619, -0.0128321, 0.0111166,
      -0.00969272, 0.0189659, -0.0160119, 0.00458659, 0.0107104, -0.000399315,
      0.000343129, 0.00117813, -2.80525e-05, 2.41086e-05, 8.2778e-05,
      -0.0450479, -0.00034974, -0.015063, 0.0655838, 0.0115163, -0.022358,
      0.0220978, 0.0583236, 0.0513097, -0.0119156, 0.00490159, 0.0116429,
      -0.0132479, -0.0146227, -0.00863565, -0.0118978, -0.000282044,
      -0.00400272, 0.0199347, 0.00139057, 0.00635067, 0.0131991, 0.000163177,
      0.00441453, 0.0159091, -0.00241207, -0.0110696, 0.0123057, 0.0171503,
      0.0119626, -0.00122682, -8.55716e-05, -0.00039083, -8.62007e-05,
      -6.0128e-06, -2.746e-05, -0.0304688, -0.954049, 0.259333, 0.0971056,
      0.255313, 0.950672, 0.983887, -0.0547431, -0.0857965, 0.0170489,
      -0.0104387, -0.036743, -0.0381557, -0.00278036, -0.0169143, -0.00177889,
      -0.04041, 0.0106552, -2.23782e-07, 2.40738e-07, 1.03401e-07, -0.000182535,
      -0.00516415, 0.00138942, 0.00125201, -0.00139237, -0.00501195,
      -0.00519809, -0.000154171, -0.00125602, 4.03664e-08, -6.04796e-08,
      -4.6768e-08, -2.38233e-09, 2.31605e-09, 1.35922e-09;

  A.BSESinglets().eigenvectors() = spsi_ref;

  Eigen::Array<votca::Index, 3, 1> steps(3, 4, 7);
  Logger log;
  double padding = 0.5;
  CubeFile_Writer writer(steps, padding, log);
  QMState state("s1");
  writer.WriteFile("test_writer.cube", A, state, false);

  auto result1 = Readcubefile("test_writer.cube");

  Eigen::VectorXd values_ref1 = Eigen::VectorXd(125);
  values_ref1 << 5, -1.688861, -1.688861, -1.688861, 3, 1.688861, 0.0, 0.0, 4,
      0.0, 1.125907, 0.0, 7, 0.0, 0.0, 0.562954, 6, 6, 0.000000, 0.000000,
      0.000000, 1, 1, 1.188861, 1.188861, 1.188861, 1, 1, -1.188861, -1.188861,
      1.188861, 1, 1, 1.188861, -1.188861, -1.188861, 1, 1, -1.188861, 1.188861,
      -1.188861, 2.086981E-04, 2.887808E-04, 1.558968E-03, 1.074590E-02,
      4.582070E-02, 9.052138E-02, 5.792652E-02, 1.047419E-03, 5.028023E-03,
      1.721917E-02, 2.991574E-02, 4.670601E-02, 7.020587E-02, 4.831243E-02,
      1.745825E-02, 2.926906E-02, 3.316943E-02, 3.386484E-02, 2.046031E-02,
      6.517114E-03, 2.506861E-03, 2.059582E-02, 3.176774E-02, 1.604636E-02,
      3.932870E-03, 6.638920E-04, 2.921923E-04, 3.913318E-04, 3.687917E-03,
      1.335639E-02, 4.059612E-02, 6.005888E-02, 4.420314E-02, 2.468902E-02,
      1.438830E-02, 4.329865E-02, 3.083563E-01, 8.948402E-01, 1.297725E+00,
      8.986854E-01, 3.050060E-01, 4.649542E-02, 4.372662E-02, 3.087091E-01,
      9.110875E-01, 1.289519E+00, 9.039197E-01, 3.112299E-01, 4.213563E-02,
      7.556830E-03, 1.696146E-02, 4.211854E-02, 6.095259E-02, 4.021124E-02,
      1.101817E-02, 1.509902E-03, 1.550671E-02, 2.323750E-02, 1.083158E-02,
      2.285980E-03, 5.195287E-04, 5.624438E-04, 6.816540E-04, 1.595056E-02,
      2.598398E-02, 3.137534E-02, 3.467681E-02, 2.139486E-02, 5.656479E-03,
      1.099170E-03, 2.376104E-03, 6.024153E-03, 1.793501E-02, 2.752360E-02,
      2.090443E-02, 8.313980E-03, 2.555200E-03, 1.039559E-03, 1.089311E-03,
      9.769998E-04, 9.895743E-04, 1.253110E-03, 1.902528E-03, 1.319557E-03;
  BOOST_CHECK_EQUAL(values_ref1.size(), result1.size());

  bool check_ref1 = values_ref1.isApprox(result1, 1e-4);
  BOOST_CHECK_EQUAL(check_ref1, true);
  if (!check_ref1) {
    std::cout << "ref" << std::endl;
    std::cout << values_ref1.transpose() << std::endl;
    std::cout << "result" << std::endl;
    std::cout << result1.transpose() << std::endl;
  }

  writer.WriteFile("test_writer2.cube", A, state, true);

  auto result2 = Readcubefile("test_writer2.cube");

  Eigen::VectorXd values_ref2 = Eigen::VectorXd(125);
  values_ref2 << 5, -1.688861, -1.688861, -1.688861, 3, 1.688861, 0.0, 0.0, 4,
      0.0, 1.125907, 0.0, 7, 0.0, 0.0, 0.562954, 6, 6, 0.000000, 0.000000,
      0.000000, 1, 1, 1.188861, 1.188861, 1.188861, 1, 1, -1.188861, -1.188861,
      1.188861, 1, 1, 1.188861, -1.188861, -1.188861, 1, 1, -1.188861, 1.188861,
      -1.188861, -2.407913E-05, 7.172120E-05, 1.055672E-03, 9.662988E-03,
      4.473291E-02, 8.988459E-02, 5.773380E-02, 5.441229E-04, -1.169403E-03,
      -8.839113E-03, -1.207413E-02, 1.775452E-02, 6.166442E-02, 4.722463E-02,
      1.637045E-02, 2.072760E-02, 4.217940E-03, -8.125022E-03, -5.597967E-03,
      3.196890E-04, 2.003565E-03, 2.040310E-02, 3.113095E-02, 1.495856E-02,
      2.849957E-03, 1.605957E-04, 7.513285E-05, 1.585549E-04, 2.605004E-03,
      2.124423E-03, -1.393775E-03, -3.726841E-03, 2.213263E-03, 1.345706E-02,
      1.330538E-02, 1.308655E-03, -2.983779E-03, -2.171675E-02, -6.024515E-02,
      -1.787102E-02, -6.334008E-03, 4.505435E-03, 1.736629E-03, -2.630904E-03,
      -5.468992E-03, -6.845145E-02, -1.263728E-02, -1.101912E-04, 1.456285E-04,
      6.473916E-03, 5.729500E-03, 1.286539E-04, -2.833127E-03, -1.778652E-03,
      -2.137889E-04, 4.269888E-04, 1.531399E-02, 2.260071E-02, 9.743778E-03,
      1.203067E-03, 1.623238E-05, 3.453845E-04, 4.488773E-04, 1.486277E-02,
      1.744253E-02, 2.423855E-03, -7.313053E-03, -4.663426E-03, -5.409484E-04,
      5.958741E-04, 1.872808E-03, -1.732707E-04, -8.123268E-03, -1.446627E-02,
      -8.047063E-03, -2.274731E-04, 1.467401E-03, 8.067823E-04, 8.722511E-04,
      4.737040E-04, -9.333764E-05, 1.653119E-04, 1.265738E-03, 1.126837E-03;

  bool check_ref2 = values_ref2.isApprox(result2, 1e-4);
  BOOST_CHECK_EQUAL(values_ref2.size(), result2.size());
  BOOST_CHECK_EQUAL(check_ref2, true);
  if (!check_ref2) {
    std::cout << "ref" << std::endl;
    std::cout << values_ref2.transpose() << std::endl;
    std::cout << "result" << std::endl;
    std::cout << result2.transpose() << std::endl;
  }

  Eigen::Array<votca::Index, 3, 1> steps2(3, 4, 8);
  Logger log2;
  double padding2 = 0.75;
  CubeFile_Writer writer2(steps2, padding2, log2);

  QMState state2("ks5");
  writer2.WriteFile("test_writer3.cube", A, state2, false);

  auto result3 = Readcubefile("test_writer3.cube");

  Eigen::VectorXd values_ref3 = Eigen::VectorXd(139);
  values_ref3 << -5, -1.938861, -1.938861, -1.938861, 3, 1.938861, 0.0, 0.0, 4,
      0.0, 1.292574, 0.0, 8, 0.0, 0.0, 0.553960, 6, 6, 0.000000, 0.000000,
      0.000000, 1, 1, 1.188861, 1.188861, 1.188861, 1, 1, -1.188861, -1.188861,
      1.188861, 1, 1, 1.188861, -1.188861, -1.188861, 1, 1, -1.188861, 1.188861,
      -1.188861, 1, 6, 2.112716E-02, 2.763840E-02, 3.295272E-02, 4.067800E-02,
      5.673489E-02, 7.426148E-02, 7.168747E-02, 4.562082E-02, 1.947294E-02,
      2.031307E-02, 1.891777E-02, 2.430098E-02, 4.626236E-02, 7.344833E-02,
      7.492110E-02, 4.564395E-02, -2.582052E-02, -4.561623E-02, -4.575029E-02,
      -2.842182E-02, -1.352742E-02, -9.685908E-03, -1.167234E-02, -1.295757E-02,
      -3.111512E-02, -4.965171E-02, -5.252702E-02, -4.180130E-02, -3.188791E-02,
      -2.717736E-02, -2.340730E-02, -1.817516E-02, 7.345894E-02, 9.688525E-02,
      8.642861E-02, 5.668347E-02, 4.476664E-02, 5.172900E-02, 5.039784E-02,
      3.318091E-02, 6.345537E-02, 4.133610E-02, -5.506930E-02, -1.972869E-01,
      -1.364185E-01, -4.877854E-03, 2.776491E-02, 1.584610E-02, -9.263052E-03,
      -1.765210E-02, 1.395015E-02, 1.399178E-01, 1.937876E-01, 4.599700E-02,
      -5.144891E-02, -7.003842E-02, -2.746550E-02, -4.161122E-02, -4.384373E-02,
      -4.172473E-02, -5.972539E-02, -9.431387E-02, -1.056719E-01, -7.917435E-02,
      8.931076E-02, 1.332886E-01, 1.279118E-01, 8.217678E-02, 4.108697E-02,
      2.076931E-02, 1.171900E-02, 6.332093E-03, 1.053504E-01, 1.553381E-01,
      1.407447E-01, 7.700291E-02, 2.204450E-02, -5.476918E-03, -1.452897E-02,
      -1.318141E-02, 6.666039E-03, 5.888243E-03, -3.754943E-03, -3.281805E-02,
      -9.484346E-02, -1.684427E-01, -1.846430E-01, -1.251738E-01, -9.284085E-03,
      -1.595010E-02, -2.654467E-02, -4.987706E-02, -9.711037E-02, -1.496463E-01,
      -1.553243E-01, -1.038165E-01;

  BOOST_CHECK_EQUAL(values_ref3.size(), result3.size());
  bool check_ref3 = values_ref3.isApprox(result3, 1e-4);
  BOOST_CHECK_EQUAL(check_ref3, true);
  if (!check_ref3) {
    std::cout << "ref" << std::endl;
    std::cout << values_ref3.transpose() << std::endl;
    std::cout << "result" << std::endl;
    std::cout << result3.transpose() << std::endl;
  }
}

BOOST_AUTO_TEST_SUITE_END()