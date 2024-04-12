#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <functional>

#define ite 10000
#define SNR 20
#define M 1
#define sigma_h2 1

#define K 2

int sc;
int number_slot;
int number_segment;
int constant_times;

using namespace std;


class Edge
{
public:
  int position;
  double snr;

  Edge(int a, double b)
    {
      position = a;
      snr = b;
    }
  ~Edge(){}

};

class Slot
{
public:
  list<Edge> s_edge;
  void add_edge(int,double);

  Slot(){}
  ~Slot(){}

};


class Device
{
public:
  list<Edge> d_edge;
  int counter;  //intra sic sareta segment su no counter
  void add_edge(int,double);
  Device(){
    counter=0;  //shokika
  }
  ~Device(){}

};


class RandomNumberGenerator
{
public:
  uint32_t seed;
  vector<uint32_t> seed_gen;
  mt19937 mt;
  random_device rng;
  int get_random_number(double a);
  double get_real_number();
  double get_normal_number();
  double exponential_number(double snr);
  void reset_seed()
    {
      seed =  rng();
      mt.seed(seed);
    }
  RandomNumberGenerator(){
      seed=rng();
    mt.seed(seed);
    generate( seed_gen.begin(), seed_gen.end(), ref(mt));

  }
  ~RandomNumberGenerator(){}

};

class Channel
{
public:
  double snr;
  double R;
  double b;
  double set_epsilon();
  
  RandomNumberGenerator s;
  Channel(){
    R = 1;
    snr = 1/(pow(10,-1*((double)SNR/10)));
    b=1; /* b=2^R-1 shannon limit */
  }
  ~Channel(){}
};


class Frame
{
public:
  int time_slot;
  int device;
  vector<Slot> f_slot;
  void add_slot();
  void add_time_slot(int a)
    {
      time_slot = a;
    }
  void add_device(int a)
    {
      device = a;
    }
  Frame(){}
  ~Frame(){}
};

class DistEdge
{
public:
  int degree;
  double coef;

  DistEdge(int a, double b)
  {
    degree = a;
    coef = b;
  }
  ~DistEdge(){}
  
};

class Dist
{
public:
  vector<DistEdge> lambda;
  vector<DistEdge> rho;
  void add_Ledge(int a, double b)
  {
    lambda.push_back(DistEdge(a,b));
  }
  void add_Redge(int a, double b)
  {
    rho.push_back(DistEdge(a,b));
  }
  
  Dist()
  {}
  ~Dist(){}
};

void Device::add_edge(int a, double b)
{
  d_edge.push_back(Edge(a, b));
}

void Slot::add_edge(int a, double b)
{
  s_edge.push_back(Edge(a, b));
}

void Frame::add_slot()
{
  Slot s;
  f_slot.push_back(s);
}

int RandomNumberGenerator::get_random_number(double a)
{
  uniform_int_distribution<int> dist(0, a);
  return dist(mt);

}

double RandomNumberGenerator::get_real_number()
{
  uniform_real_distribution<double> dist(0, 1);
  return dist(mt);
}

double RandomNumberGenerator::exponential_number(double snr)
{
  exponential_distribution<> dist(1.0/snr);
  return dist(mt);
}



double Channel::set_epsilon()
{
  double eps = 0;
  if(snr>(double)(b))eps=0.0;
  else eps=1.0;
  return eps;
}
