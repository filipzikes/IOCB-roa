/******************************************************************************
 * Program: GroupSort
 * Author: Filip Zikes
 * Email:  zikesfilip@gmail.com
 * Institution: UOCHB/IOCB Prague
 * Date: January 15, 2022
 * Version: 2.0
 * 
 * COMPILATION: g++ -o group_sort  group_sort.cpp -lm
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <math.h>
#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include <set>
#include <algorithm>


typedef float angType;
#define minType 0.00001

/********************************
 * __________VARIABLES__________
********************************/
std::vector<std::pair<int, std::vector<angType> > > snapshots;
std::set<std::pair<int, int> > grouped;
FILE *fin, *fout;
angType value;
int angles_num, angle_i, i, dist, offset;
char line[200], symbol[1];
bool multi_files = false; bool intervals = false; bool check_int = false; bool grpInt;

/********************************
 * __________FUNCTIONS__________
********************************/
void exitProg(){
  std::cout << "\nFor more help type \"group_sort -help\".\n";
  exit(0);
}
void openWFile(std::string str){
  if ((fout=fopen(str.c_str(), "w"))==NULL) {std::cout << "ERROR creating output file.\n"; exitProg();}
}
namespace to {
  template < typename T > std::string str ( const T& _value ) {
    std::ostringstream _str; _str << _value;
    return _str.str();
  }
  template < class T > T next(T it, typename std::iterator_traits<T>::difference_type n = 1) {
    std::advance(it, n);
    return it;
  }
}

/********************************
 * _________INT_MAIN_()_________
********************************/
int main(int argc, char** argv){
  //-HELP
    #define HELP1 "Usage:\n > group_sort [AnglesParameterFile] [PRN_InputFile] [multifileOutput (y/n)] [angleOverflow (y/n)] [intervalsFile (y/n)]"
    #define HELP2 "\n > group_sort -help\n\nAngles parameter file:\n"
    #define HELP3 " > [Total number of angles] [useIntervals (y/n)]\n > [angle 1 delimiter] [angle 1 delimiter] ...\n > [angle 2 delimiter] [angle 2 delimiter] ...\n >  ...\n\n"
    #define HELP4 "PRN Input File:\n > [Conformer number] [angle 1 value] [angle_2 value] ...\n > [Conformer number] [angle 1 value] [angle_2 value] ...\n > ...\n"
    if (argc>1) if (std::string(argv[1])=="-help") {std::cout << HELP1 << HELP2 << HELP3 << HELP4; exit(0);};

  /*****************************
   * ReadingParameterFile
   * USING: fopen, fget, sscanf
  *****************************/
    if (argc>1) {if ((fin=fopen(argv[1] , "r"))==NULL) {std::cout << "ERROR loading parameter file.\n"; exitProg();}
                }else{std::cout << "Parameter file:\n";std::string s;std::cin >> s;
                if ((fin=fopen(s.c_str() , "r"))==NULL) {std::cout << "ERROR loading parameter file.\n"; exitProg();}}
    if (fgets(line, 200, fin)) {sscanf(line, "%d %s", &angles_num, symbol);}else{exitProg();}
    if (symbol[0]=='y') {intervals = true; std::cout << "Using angle min-max intervals.\n";}

    std::set<angType> angles[angles_num]; angle_i = 0;
    int angles_size[angles_num];

    while(fgets(line, 200, fin) && angle_i<angles_num){
      std::string data(line);
      while (sscanf(data.c_str(), "%g%n", &value, &offset)>0) {while(angles[angle_i].find(value)!=angles[angle_i].end()){value+=minType;}; angles[angle_i].insert(value); data = data.substr(offset, data.size()-offset);}
      angles_size[angle_i] = angles[angle_i].size();
      if (intervals && angles_size[angle_i]%2==1) {angles[angle_i].insert(180); angles_size[angle_i]=angles[angle_i].size();}
      angle_i++;
    }
    if (angle_i<angles_num) {
      std::cout << (angles_num-angle_i) << " lines with angles data are missing.\n";
      std::cout << "Do you want to continue? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)!='y') exitProg();
      angles_num = angle_i;
    }
    while (fgets(line, 200, fin)) {angle_i++;}
    if (angle_i>angles_num) std::cout << "Found " << (angle_i-angles_num) << " more lines with surplus angles data.\n";
    fclose(fin);

  /*****************************
   * SettingParameters
   * USING: received arguments
  *****************************/
    if (argc>3){
      if (argv[3][0]=='y') multi_files=true;
    }else{
      std::cout << "Generate multiple group output files? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)=='y') multi_files=true;
    }
    if (argc>4){
      if (argv[4][0]=='y') check_int = true;
    }else{
      std::cout << "Check for angles overflow? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)=='y') check_int=true;
    }
    if (argc>5){
      if (argv[5][0]=='y') grpInt = true;
    }else{
      std::cout << "Generate file of group intervals? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)=='y') grpInt=true;
    }
    if (check_int){
      for(i=0; i<angles_num; i++){
        if(angles[i].lower_bound(-180)!=angles[i].begin() || angles[i].upper_bound(180)!=angles[i].end()){
          std::vector<angType> angle_change[2];
          for(std::set<angType>::iterator it = angles[i].begin(); it!=angles[i].end(); it++){
            angType val = *it;
            angType div = fmod(val+180, 360);
            div = (div<0) ? div+180 : div-180;
            if (div!=val){
              angle_change[0].push_back(val);
              angle_change[1].push_back(div);
            }else{
              it=angles[i].upper_bound(180);
            }
          }
          for(int j=0; j<std::min(angle_change[0].size(), angle_change[1].size()); j++){
            angles[i].erase(angles[i].find(angle_change[0][j]));
            angles[i].insert(angle_change[1][j]);
          }
        }
      }
    }

  /*****************************
   * Print group intervals
   * USING: fopen, fprintf
  *****************************/
    if(grpInt){
      std::string ang_format; bool int_loop = true;
      if(intervals){ang_format = " <%.4g; %.4g>";}else{ang_format = " (%.4g; %.4g>";}
      int ang_index[angles_num]; int ang_index_next[angles_num]; int index; int max_index = 1;
      for (i=0; i<angles_num; i++){if(intervals){max_index=max_index*angles_size[i]/2;}else{max_index=max_index*angles_size[i];};ang_index[i] = 0; ang_index_next[i] = 1 % angles_size[i];}
      max_index--;
      openWFile("GROUP_INTERVALS.PRN");
      fprintf(fout, "-1 \"undefined interval\"");

      index = 1; fprintf(fout, "\n%d", 1);
      for(i=0; i<angles_num; i++){
        fprintf(fout, ang_format.c_str(), *(to::next(angles[i].begin(), ang_index[i])), *(to::next(angles[i].begin(), ang_index_next[i])) );
      }

      while(int_loop){
        for(i=0; i<angles_num; i++){
          if(intervals){
            ang_index[i] = (ang_index[i]+2) % angles_size[i];
            ang_index_next[i] = (ang_index[i]+1) % angles_size[i];
          }else{
            ang_index[i] = ang_index_next[i];
            ang_index_next[i] = (ang_index[i]+1) % angles_size[i];
          }
          if(ang_index[i]!=0) i = angles_num;
        }
        index++; if(index>max_index) int_loop = false;
        fprintf(fout, "\n%d", index);
        for(i=0; i<angles_num; i++){
          fprintf(fout, ang_format.c_str(), *(to::next(angles[i].begin(), ang_index[i])), *(to::next(angles[i].begin(), ang_index_next[i])) );
        }
      }
      fclose(fout);
    }

  /*****************************
   * ReadingInputFile
   * USING: fopen, fget, sscanf
  *****************************/
    if (argc>2) {if ((fin=fopen(argv[2] , "r"))==NULL) {std::cout << "ERROR loading input file.\n"; exitProg();}
                }else{std::cout << "Input file:\n";std::string s;std::cin >> s;
                if ((fin=fopen(s.c_str() , "r"))==NULL) {std::cout << "ERROR loading input file.\n"; exitProg();}}
    while(fgets(line, 200, fin)){
      
      std::pair<int, std::vector<angType> > new_snapshot;
      std::string data(line);
      if (sscanf(data.c_str(), "%d%n", &i, &offset)==0) {std::cout << "ERROR wrong format of input file.\n"; exitProg();}
      data = data.substr(offset, data.size()-offset);
      new_snapshot.first = i;
      
      while (sscanf(data.c_str(), "%g%n", &value, &offset)>0) {
        if (check_int){if (value<-180||value>180){value = fmod(value+180, 360); value = (value<0) ? value+180 : value-180;}}
        new_snapshot.second.push_back(value); data = data.substr(offset, data.size()-offset);
      }
      //snapshots.push_back(new_snapshot); //ADDS-ELEMENT-TO-THE-END
      snapshots.insert(snapshots.begin(), new_snapshot); //ADDS-ELEMENT-TO-THE-BEGIN
    }
    fclose(fin);

  /*******************************
   * GroupSorting
   * USING: lower_bound, distance
  *******************************/
    if(intervals){
      while(snapshots.size()>0){
        std::pair<int, std::vector<angType> > current_snapshot = snapshots.back();
        int group_index = 0; int group_product = 1;
        angle_i = current_snapshot.second.size();
        offset = std::min(angle_i, angles_num);

        if (angle_i < angles_num) {offset=0; group_index = -2;}
        for(i=0; i<offset; i++){
          std::set<angType>::iterator it = angles[i].lower_bound(current_snapshot.second[i]);
          if (it==angles[i].end()) it = angles[i].begin();
          dist = std::distance(angles[i].begin(), it);
          if (dist%2==1){
            group_index = group_index + (dist/2) * group_product;
            group_product = group_product * angles_size[i]/2;
          }else if(*it==current_snapshot.second[i]) {
            group_index = group_index + (dist/2) * group_product;
            group_product = group_product * angles_size[i]/2;
          }else{
            i=offset;
            group_index = -2;
          }
        }
        grouped.insert(std::make_pair(group_index+1, current_snapshot.first));
        snapshots.pop_back();
      }
    }else{
      while(snapshots.size()>0){
        std::pair<int, std::vector<angType> > current_snapshot = snapshots.back();
        int group_index = 0; int group_product = 1;
        angle_i = current_snapshot.second.size();
        offset = std::min(angle_i, angles_num);

        if (angle_i < angles_num) {offset=0; group_index = -2;}
        for(i=0; i<offset; i++){
          std::set<angType>::iterator it = angles[i].lower_bound(current_snapshot.second[i]);
          if (it==angles[i].end()) it = angles[i].begin();
          group_index = group_index + ((std::distance(angles[i].begin(), it)+angles_size[i]-1) % angles_size[i]) * group_product;
          group_product = group_product * angles_size[i];
        }
        
        grouped.insert(std::make_pair(group_index+1, current_snapshot.first));
        snapshots.pop_back();
      }
    }

  /***********************
   * WritingOutputFile
   * USING: fopen, fprintf
  ************************/
    if (multi_files){
      std::set<std::pair<int, int> >::iterator it = grouped.begin();
      if (it != grouped.end()) {
        offset = it->first; openWFile(std::string("GROUP_")+to::str(offset)+std::string(".PRN"));
        fprintf(fout, "%d\n", it->second);it++;
      }

      while(it != grouped.end()){
        if(offset!=it->first){
          offset = it->first;
          fclose(fout);
          openWFile(std::string("GROUP_")+to::str(offset)+std::string(".PRN"));
        }
        fprintf(fout, "%d\n", it->second);
        it++;
      }
      fclose(fout);
      std::cout << "Group files succesfully written.\n";
    }else{
      std::string name_out = "GROUPS.PRN";
      std::set<std::pair<int, int> >::iterator it = grouped.begin();
      openWFile(name_out);
      if (it != grouped.end()) {offset = it->first; fprintf(fout, "GROUP %d\n", offset); fprintf(fout, "%d\n", it->second);it++;}

      while(it != grouped.end()){
        if(offset!=it->first){
          offset = it->first;
          fprintf(fout, "\nGROUP %d\n", offset);
        }
        fprintf(fout, "%d\n", it->second);
        it++;
      }
      fclose(fout);
      std::cout << "File GROUPS.PRN succesfully written.\n";
    }

  /***********************
   * _____END_OF_CODE_____
  ************************/
    return(0);
}
