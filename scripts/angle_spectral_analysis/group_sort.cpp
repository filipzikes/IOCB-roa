/******************************************************************************
 * Program: GroupSort
 * Author: Filip Zikes
 * Email:  zikesfilip@gmail.com
 * Institution: UOCHB/IOCB Prague
 * Date: May 22, 2022
 * Version: 2.1
 * 
 * COMPILATION: g++ -o group_sort  group_sort.cpp -lm
******************************************************************************/

#define ANG float

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

/********************************
 * _________DEFINTIONS__________
********************************/
typedef ANG angType;
namespace to {
  template < typename T > std::string str ( const T& _value );
  template < class T > T next(T it, typename std::iterator_traits<T>::difference_type n = 1);
  void exit_prog();
  void write_file(std::string str);
  std::vector<int> int_vect();
}
namespace parameters {
  struct parameters {
    int angles_num, min_group_size;
    bool multi_files = false;
    bool intervals = false;
    bool check_int = false;
    bool grp_int = false;
    std::string script_name;
  };
  struct groups {
    std::vector<angType> *angles;
    int *size;
    
    void set_size(int n){
      angles = new std::vector<angType>[n];
      size = new int[n];
    }
  };
  void load(int argc, char** argv);
  void ask(int argc, char** argv);
}
namespace intervals {
    void check();
    void print();
    void print_all();
}
namespace spectra {
    struct groups {
      std::vector < std::pair<int, std::vector<angType>> > snapshots;
      std::map <int, std::vector<int> > sort;
    };
    void load(int argc, char** argv);
    void sort(std::vector<ANG> sym);
    void print();
}
namespace additions {
  void group_conformers();
  void filter_symmetry();   //TODO: REPAIR PRINTED INTERVALS (#todo)
  void check_size();
};

/********************************
 * __________VARIABLES__________
********************************/
parameters::parameters prm;
parameters::groups ang;
spectra::groups grp;
FILE *fin, *fout; char line[200], symbol[1];

/********************************
 * _________INT_MAIN_()_________
********************************/
int main(int argc, char** argv){
    parameters::load(argc, argv);
    parameters::ask(argc, argv);

    intervals::check();

    spectra::load(argc, argv);
    additions::filter_symmetry();   //DOES: spectra::sort(_v); _v = {1, -1}
      additions::check_size();

    intervals::print();             //INSTEAD OF: intervals::print_all();
    spectra::print();

    return 0;
}

/********************************
 * __________NAMESPACE__________
********************************/
#define HELP1 "Usage:\n > group_sort [AnglesParameterFile] [PRN_InputFile] [multifileOutput (y/n)] [angleOverflow (y/n)] [intervalsFile (y/n)]"
#define HELP2 "\n > group_sort -help\n\nAngles parameter file:\n"
#define HELP3 " > [Total number of angles] [useIntervals (y/n)]\n > [angle 1 delimiter] [angle 1 delimiter] ...\n > [angle 2 delimiter] [angle 2 delimiter] ...\n >  ...\n\n"
#define HELP4 "PRN Input File:\n > [Conformer number] [angle 1 value] [angle_2 value] ...\n > [Conformer number] [angle 1 value] [angle_2 value] ...\n > ...\n"
#define STR(_str) std::string(_str)

template < typename T > std::string to::str ( const T& _value ) {
    std::ostringstream _str; _str << _value;
    return _str.str();
  }
template < class T > T to::next(T it, typename std::iterator_traits<T>::difference_type n) {
    std::advance(it, n);
    return it;
  }
void to::exit_prog(){
    std::cout << "\nFor more help type \"group_sort -help\" or \"" + prm.script_name + " -help\".\n";  //USE DYNAMIC COMMAND NAME (#todo)
    exit(0);
  }
void to::write_file(std::string str){
  if ((fout=fopen(str.c_str(), "w"))==NULL) {std::cout << "ERROR creating output file.\n"; to::exit_prog();}
}
std::vector<int> to::int_vect(){
  std::vector<int> _v;
  return _v;
}

/********************************
 *           PARAMETERS
********************************/
void parameters::load(int argc, char** argv){
  prm.script_name = STR(argv[0]);
  if (argc>1) if (std::string(argv[1])=="-help") {std::cout << HELP1 << HELP2 << HELP3 << HELP4; exit(0);};
  if (argc>1) {if ((fin=fopen(argv[1] , "r"))==NULL) {std::cout << "ERROR loading parameter file.\n"; to::exit_prog();}
          }else{std::cout << "Parameter file:\n";std::string s;std::cin >> s;
          if ((fin=fopen(s.c_str() , "r"))==NULL) {std::cout << "ERROR loading parameter file.\n"; to::exit_prog();}}
  if (fgets(line, 200, fin)) {sscanf(line, "%d %s", &(prm.angles_num), symbol);}else{to::exit_prog();}
  if (symbol[0]=='y') {prm.intervals = true; std::cout << "Using angle min-max intervals.\n";}

//READING ANGLE INTERVALS (#info)
  ang.set_size(prm.angles_num);
  int a, offset; angType value;

  for (a = 0; fgets(line, 200, fin) && a < prm.angles_num; a++){
      std::string data(line);

    //READING LINE STRING (#C)
      while (sscanf(data.c_str(), "%g%n", &value, &offset)>0) {
        ang.angles[a].push_back(value);
        data = data.substr(offset, data.size()-offset);
      }
      ang.size[a] = ang.angles[a].size();

  }

//CHECKING NUMBER OF ANGLES (#info)
  if (a < prm.angles_num) {
      std::cout << (prm.angles_num - a) << " lines with angles data are missing.\n";
      std::cout << "Do you want to continue? (y/n)\n";
      std::string read_character; std::cin >> read_character;
      if (read_character.at(0)!='y') to::exit_prog(); prm.angles_num = a;
    }
    while (fgets(line, 200, fin)) {a++;}
    if (a > prm.angles_num) std::cout << "Found " << (a - prm.angles_num) << " more lines with surplus angles data which won't be used.\n";
    fclose(fin);
}
void parameters::ask(int argc, char** argv){
  if (argc>3){
      if (argv[3][0]=='y') prm.multi_files=true;
    }else{
      std::cout << "Generate multiple group output files? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)=='y') prm.multi_files=true;
    }
    if (argc>4){
      if (argv[4][0]=='y') prm.check_int = true;
    }else{
      std::cout << "Check for angles overflow? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)=='y') prm.check_int=true;
    }
    if (argc>5){
      if (argv[5][0]=='y') prm.grp_int = true;
    }else{
      std::cout << "Generate file of group intervals? (y/n)\n";
      std::string read_character;
      std::cin >> read_character;
      if (read_character.at(0)=='y') prm.grp_int=true;
    }
    if (argc>6){
        prm.min_group_size = atoi(argv[6]);
    }else{
        prm.min_group_size = 0;
    }
}

/********************************
 *           INTERVALS
********************************/
void intervals::check(){
    for (int a = 0; a < prm.angles_num; a++){

    //CHECK IF IN INTERVAL (#C)
      for (int i = 0; i < ang.size[a]; i++) {
        if (prm.check_int){
          if ( (180-ang.angles[a][i])*(ang.angles[a][i]+180) < 0 ) {
            angType val = ang.angles[a][i];
            val = fmod(val+180, 360);
            val = (val<0) ? val+180 : val-180;
            ang.angles[a][i] = val;
          }
        }
      }

    //CHECKING CORRECT INTERVAL ENTRY (#info)
      if (prm.intervals) {
        if (ang.size[a]%2==1) {std::cout << "The number of values defining intervals has to be even.\n"; to::exit_prog();};

        int i, gt, lt, eq; gt = 0; lt = 0; eq = 0;
        for(i = 0; i < ang.angles[a].size()-1; i++){
          if (ang.angles[a][i+1] > ang.angles[a][i]){ gt++;
          } else if(ang.angles[a][i+1] < ang.angles[a][i]){ lt++; std::cout << ang.angles[a][i+1] << ":" << ang.angles[a][i] << "\n";
          } else {eq++;};
        }
        if (ang.angles[a][0] > ang.angles[a][i]){ gt++;
          } else if(ang.angles[a][0] < ang.angles[a][i]){ lt++;
          } else {eq++;};
        if (lt > 1) { //1 possible interval swap (#warn)   //(gt > 1 && ...) affects sorting algorithm [lt operator in spectra::sort] (#todo)
          std::cout << "The intervals should be ordered in one way, there's only 1 possible transition over 180/-180 degrees.\n"; to::exit_prog();};
      }
      
    }
}
void intervals::print(){
  int type = prm.intervals ? 2 : 1;

  if (prm.grp_int) {
    int a, ang_index[prm.angles_num];

    to::write_file("GROUP_INTERVALS.PRN");
    fprintf(fout, "-1 \"undefined interval\"\n");

    for (auto const& group : grp.sort) {
      ang_index[0] = group.first;
      fprintf(fout, "%d", group.first);

      for (a = 0; a < prm.angles_num - 1; a++) {
        ang_index[a + 1] = ang_index[a] / ang.size[a] * type;
        ang_index[a] = ang_index[a] % ( ang.size[a] / type );
        fprintf(fout, " <%.4g; %.4g>", ang.angles[a][ang_index[a]], ang.angles[a][(ang_index[a]+1) % (ang.size[a]+1)]);
      }
      fprintf(fout, "\n");
    }

    fclose(fout);
  }
}
void intervals::print_all(){
  if (prm.grp_int) {
    
    bool int_loop; int a, index, max_index, ang_index[prm.angles_num], ang_index_next[prm.angles_num];
    std::string ang_format = " (%.4g, %.4g)"; int_loop = true; max_index = 1;

  //SET MAX INDEX BY SIZES OF ANG INTERVALS, DEFINE THE INITIAL INDEX VALUES (#info)
    for (a = 0; a < prm.angles_num; a++) {
      if (prm.intervals) {
        max_index = max_index * ang.size[a] / 2;
      } else {
        max_index = max_index * ang.size[a];
      }
      ang_index[a] = 0; ang_index_next[a] = 1 % ang.size[a];
    }
    max_index--;
    
    to::write_file("GROUP_INTERVALS.PRN");
    fprintf(fout, "-1 \"undefined interval\"");

  //PRINT FIRST GROUP (#info)
    index = 1; fprintf(fout, "\n%d", 1);
    for (a = 0; a < prm.angles_num; a++) {
      fprintf(fout, ang_format.c_str(), ang.angles[a][ang_index[a]], ang.angles[a][ang_index_next[a]] );
    }
    
  //LOOPING THROUGH ALL POSSIBLE DIGITS (#C)
    while (int_loop) {
        for(a = 0; a < prm.angles_num; a++){
          if (prm.intervals) {
            ang_index[a] = (ang_index[a] + 2) % ang.size[a];
            ang_index_next[a] = (ang_index[a] + 1) % ang.size[a];
          }else{
            ang_index[a] = ang_index_next[a];
            ang_index_next[a] = (ang_index[a] + 1) % ang.size[a];
          }
          if ( ang_index[a] != 0 ) a = prm.angles_num;
        }
        index++; if ( index > max_index ) int_loop = false;
        
        fprintf(fout, "\n%d", index);
        for(a = 0; a < prm.angles_num; a++){
          fprintf(fout, ang_format.c_str(), ang.angles[a][ang_index[a]], ang.angles[a][ang_index_next[a]] );
        }
    }

    fclose(fout);
  }
}

/********************************
 *            SPECTRA
********************************/
void spectra::load(int argc, char** argv){
  if (argc>2) {if ((fin=fopen(argv[2] , "r"))==NULL) {std::cout << "ERROR loading input file.\n"; to::exit_prog();}
                }else{std::cout << "Input file:\n";std::string s;std::cin >> s;
                if ((fin=fopen(s.c_str() , "r"))==NULL) {std::cout << "ERROR loading input file.\n"; to::exit_prog();}}
  
  int i, offset; angType value;

  while(fgets(line, 200, fin)){
      std::pair<int, std::vector<angType>> new_snapshot;
      std::string data(line);
      if (sscanf(data.c_str(), "%d%n", &i, &offset)==0) {std::cout << "ERROR wrong format of input file.\n"; to::exit_prog();}

      data = data.substr(offset, data.size()-offset);
      new_snapshot.first = i;
    
    //READ LINE AND CHECK INTERVALS (#C)
      while (sscanf(data.c_str(), "%g%n", &value, &offset)>0) {
        if (prm.check_int) { if ( (180-value)*(value+180) < 0 ) { value = fmod(value+180, 360); value = (value<0) ? value+180 : value-180;}}
        new_snapshot.second.push_back(value); data = data.substr(offset, data.size()-offset);
      }
    
    //ADD ELEMENT TO THE END (#info)
      grp.snapshots.push_back(new_snapshot); 
    }
    fclose(fin);
}
void spectra::sort(std::vector<ANG> sym){
  int a, i, group_index, group_product, angles, angle_index, offset, type; bool search_loop;
  std::set<int> symmetry;
  type = prm.intervals ? 2 : 1;
  
  //LOOP THROUGH SNAPSHOTS (#info)
    for (i = 0; i < grp.snapshots.size(); i++) {
      angles = grp.snapshots[i].second.size(); offset = std::min(angles, prm.angles_num);
      if (angles < prm.angles_num) { offset = 0; group_index = -2; goto INSERTION;}
    
  //LOOP THROUGH SYMMETRIES (#info, #additional)
      for (ANG s : sym) {
        
        group_index = 0; group_product = 1;

    //LOOP THROUGH ANGLES (#info)
      for (a = 0; a < offset; a++) {
        if ( ( s * grp.snapshots[i].second[a] ) < ang.angles[a][0] ) {
          angle_index = ang.size[a]; search_loop = false;
        } else {
          search_loop = true; angle_index = 0;
        }
        
      //ITERATE TILL THE CORRESPONDING ANGLE, CALCULATE ITS RANK (#C)
        while (search_loop && angle_index < ang.size[a] ) {
          search_loop = ( ( s * grp.snapshots[i].second[a] ) > ang.angles[a][angle_index] );
          if (search_loop) angle_index += type;
        }
        angle_index -= type;

        if ( (ang.angles[a][(angle_index+1) % ang.size[a]] - ( s * grp.snapshots[i].second[a] )) * (( s * grp.snapshots[i].second[a] ) - ang.angles[a][angle_index]) < 0 ) {
        //CHECK IF TRANSITION OVER -180/180
        //OVERLOADED SYNTAX OF INTERVAL CONDITION (#warn, #C, #todo)
          if (ang.angles[a][(angle_index+1) % ang.size[a]] < ang.angles[a][angle_index]) {
            group_index = group_index + angle_index / type * group_product;
            group_product = group_product * ang.size[a] / type;
          } else {
            group_index = -2;
            a = offset;
          }
        } else {
          group_index = group_index + angle_index / type * group_product;
          group_product = group_product * ang.size[a] / type;
        }
      }
      
        symmetry.insert(group_index);
      }

      //GET MINIMUM ELEMENT FROM SET (#C++)
      if ( !symmetry.empty() ) {
        std::set<int>::iterator it = symmetry.upper_bound(-2);  //*(returns an iterator to the first element greater than the given key)
        if ( it != symmetry.end() ) {group_index = *it;} else group_index = -2;
      }
      symmetry.clear();

      INSERTION:
    //INSERT TO VECTOR IN MAP (#C++)
      if ( grp.sort.find(++group_index) == grp.sort.end() ) grp.sort.insert(std::make_pair(group_index, to::int_vect() ));
      grp.sort[group_index].push_back(grp.snapshots[i].first);
    }
}
void spectra::print(){
  if (! prm.multi_files) to::write_file(STR("GROUPS.PRN"));

  //PRINT VECTOR & MAP ITERATION (#C++)
    for (auto const& group : grp.sort) {
      if (prm.multi_files) { to::write_file( STR("GROUP_") + to::str(group.first) + STR(".PRN") );
      } else { fprintf(fout, "GROUP %d\n", group.first); }

      for (int snap : group.second) {
        fprintf(fout, "%d\n", snap);
      }

      if (prm.multi_files) { fclose(fout);
      } else { fprintf(fout, "\n"); }
    }
    
    if (prm.multi_files) { std::cout << "Group files succesfully written.\n";
    } else { fclose(fout); std::cout << "File GROUPS.PRN succesfully written.\n"; }
    
}

/********************************
 *           ADDITIONS
********************************/
void additions::group_conformers(){}
void additions::filter_symmetry(){
  std::vector<ANG> vec;
  vec.push_back(1.0F); vec.push_back(-1.0F);
  spectra::sort(vec);
}
void additions::check_size(){
    std::vector<int> to_erase;

  //CHECK MINIMAL LIMIT IN EACH GROUP (#C++)
    for (auto const& group : grp.sort) {
      if (group.second.size() < prm.min_group_size) {
        to_erase.push_back(group.first);
      }
    }

  //ERASE INAPPROPRIATE GROUPS (#C++)
    for (int key : to_erase) {
      grp.sort.erase(key);
    }
}
