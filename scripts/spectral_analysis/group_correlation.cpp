/******************************************************************************
 * Program: GroupCorrelation
 * Author: Filip Zikes
 * Email:  zikesfilip@gmail.com
 * Institution: UOCHB/IOCB Prague
 * Date: February 13, 2022
 * Version: 1.0
 * 
 * COMPILATION: g++ -o group_correlation group_correlation.cpp
******************************************************************************/

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <vector>
#include <map>
#include <iterator>

typedef float SPARG;
typedef float SPINT;
#define SPARG_FROM 50
#define SPARG_STEP 1
#define SPARG_TO   4000

/********************************
 * __________VARIABLES__________
********************************/
std::string prefix, suffix;
std::vector<SPINT> referential, spectrum;
std::vector<int> groups;
std::map<SPARG, SPINT> m_ref, m_spc;
std::map<float, int> coefficients;
std::map<SPARG, SPINT>::iterator m_it;
SPARG m1; SPARG mx; SPINT m2;
FILE *fin, *fout;
char line[200];
int num;

/********************************
 * __________FUNCTIONS__________
********************************/
void exitProg(){
  std::cout << "\nFor more help type \"group_correlation -help\".\n";
  exit(0);
}
void openWFile(std::string str){
  if ((fout=fopen(str.c_str(), "w"))==NULL) {std::cout << "ERROR creating output file.\n"; exitProg();}
}
template < typename T> float correlation(std::vector<T> u, std::vector<T> v){
	int n = (u.size()<v.size()) ? u.size() : v.size();
	T sum_u = 0, sum_v = 0, sum_uv = 0;
	T squareSum_u = 0, squareSum_v = 0;
	typename std::vector<T>::const_iterator ut = u.begin(), vt = v.begin();
	while(ut!=u.end() && vt!=v.end()){
		sum_u = sum_u + (*ut);
		sum_v = sum_v + (*vt);
		sum_uv = sum_uv + (*ut) * (*vt);
		squareSum_u = squareSum_u + (*ut) * (*ut);
		squareSum_v = squareSum_v + (*vt) * (*vt);
		ut++; vt++;
	}
	return (float)(n * sum_uv - sum_u * sum_v) / sqrtf((float)(n * squareSum_u - sum_u * sum_u) * (n * squareSum_v - sum_v * sum_v));
}
namespace to {
  template < typename T > std::string str ( const T& _value ) {
    std::ostringstream _str; _str << _value;
    return _str.str();
  }
}

/********************************
 * _________INT_MAIN_()_________
********************************/
int main(int argc, char** argv){
  //-HELP
    #define HELP1 "Usage:\n > group_correlation [GroupsParameterFile] [ReferentialSpectrum] [GroupSpectrumFileLocationPrefix] [GroupSpectrumFileLocationSuffix]"
    #define HELP2 "\n > group_correlation -help\n\nGroups Parameter File:\n"
    #define HELP3 " > [group 1 code]\n > [group 2 code]\n >  ...\n\n"
    #define HELP4 "PRN Spectrum File:\n > [Frequency] [Intensity]\n > [Frequency] [Intensity]\n > ...\n"
    if (argc>1) if (std::string(argv[1])=="-help") {std::cout << HELP1 << HELP2 << HELP3 << HELP4; exit(0);};


  /*****************************
   * ReadingParameterFile
   * USING: fopen, fget, sscanf
  *****************************/
    if (argc>1) {if ((fin=fopen(argv[1] , "r"))==NULL) {std::cout << "ERROR loading parameter file.\n"; exitProg();}
                }else{std::cout << "Parameter file:\n";std::string s;std::cin >> s;
                if ((fin=fopen(s.c_str() , "r"))==NULL) {std::cout << "ERROR loading parameter file.\n"; exitProg();}}
	while(fgets(line, 200, fin)){
	  sscanf(line, "%d", &num);
	  groups.push_back(num);
    }
    fclose(fin);

  /*****************************
   * ReadingInputFile
   * USING: fopen, fget, sscanf
  *****************************/
    if (argc>2) {if ((fin=fopen(argv[2] , "r"))==NULL) {std::cout << "ERROR loading input file.\n"; exitProg();}
                }else{std::cout << "Input file:\n";std::string s;std::cin >> s;
                if ((fin=fopen(s.c_str() , "r"))==NULL) {std::cout << "ERROR loading input file.\n"; exitProg();}}
    while(fgets(line, 200, fin)){
	  if (sscanf(line, "%g %g", &m1, &m2)==0) {std::cout << "ERROR wrong format of input file.\n"; exitProg();}
      m_ref.insert(std::make_pair(m1, m2));
    }
    fclose(fin);
	for(mx = SPARG_FROM; mx < SPARG_TO; mx += SPARG_STEP){
	  m_it = m_ref.lower_bound(mx); if(m_it==m_ref.end()) m_it--;
	  referential.push_back(m_it->second);
	}
	m_it = m_ref.lower_bound(mx); if(m_it==m_ref.end()) m_it--;
	referential.push_back(m_it->second);

  /*******************************
   * Cor. coef. for each group
   * USING: lower_bound
  *******************************/
 	if (argc>3) {prefix = std::string(argv[3]);}else{std::cout << "Group Spectrum File Location Prefix:\n";std::cin >> prefix;}
	if (argc>4) {suffix = std::string(argv[4]);}else{std::cout << "Group Spectrum File Location Suffix:\n";std::cin >> suffix;}
 	for(std::vector<int>::const_iterator it = groups.begin(); it != groups.end(); it++){

	  /*****************************
	   * ReadingGroupInputFile
	   * USING: fopen, fget, sscanf
	  *****************************/
	 	m_spc.clear(); spectrum.clear();
		if ((fin=fopen(std::string(prefix+to::str(*it)+suffix).c_str(), "r"))==NULL) {std::cout << std::string(prefix+to::str(*it)+suffix) << "ERROR loading input file.\n"; exitProg();}
		while(fgets(line, 200, fin)){
	  	  if (sscanf(line, "%g %g", &m1, &m2)==0) {std::cout << "ERROR wrong format of input file.\n"; exitProg();}
      	  m_spc.insert(std::make_pair(m1, m2));
    	}
    	fclose(fin);
		for(mx = SPARG_FROM; mx < SPARG_TO; mx += SPARG_STEP){
		  m_it = m_spc.lower_bound(mx); if(m_it==m_spc.end()) m_it--;
		  spectrum.push_back(m_it->second);
		}
		m_it = m_spc.lower_bound(mx); if(m_it==m_spc.end()) m_it--;
		spectrum.push_back(m_it->second);

	  /*****************************
	   * Eval correlation coefficient
	   * USING: correlation
	  *****************************/
	 	coefficients.insert(std::make_pair(correlation(referential, spectrum), (*it)));

	  /***********************
   	   * _____END_OF_LOOP_____
  	  ************************/
	}

  /***********************
   * WritingOutputFile
   * USING: fopen, fprintf
  ************************/
 	std::string name_out = "COEFS.PRN";
    openWFile(name_out);
	for(std::map<float, int>::reverse_iterator it = coefficients.rbegin(); it != coefficients.rend(); it++){
	  fprintf(fout, "%d %g\n", it->second, it->first);
	}
    fclose(fout);
    std::cout << "File COEFS.PRN succesfully written.\n";


  /***********************
   * _____END_OF_CODE_____
  ************************/
    return(0);
}
