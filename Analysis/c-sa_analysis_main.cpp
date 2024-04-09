
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <functional>
#include <fstream>

using namespace std;

#define SNR 20       /* SNR [dB] */
#define M 14        /* M+1 collision wihtin a slot */
#define sigma_h2 1 /* variance of Rayleigh fading */
#define IS 2 /* number of information segment */

using namespace std;

class Slotted_ALOHA
{
public:
  vector<double> epsilon_n;
  double p;
  double q;
  double G;
  double snr;
  double theta;
  double b;
  Slotted_ALOHA(){
    for(int i=0; i<M+1; i++)
      {
        epsilon_n.push_back(0);
      }
      theta=1.0;
      snr = 1/(pow(10,-1*((double)SNR/10))); /* SNR (linear value) */
  }
  ~Slotted_ALOHA(){}
};

class Edge
{
public:
  int degree;
  double coef;
  Edge(int a, double b)
  {
    degree = a;
    coef = b;
  }
  ~Edge(){}
};

class Dist
{
public:
  vector<Edge> lambda;
  vector<Edge> rho;
  void add_Ledge(int a, double b)
  {
    lambda.push_back(Edge(a,b));
  }
  void add_Redge(int a, double b)
  {
    rho.push_back(Edge(a,b));
  }
  Dist()
  {}
  ~Dist(){}
};

double function_theta(double snr, double R)
{
  return (pow(2.0, R)-1);
}

double factorial(int n)
{
 double x=1;
    for (int i = 1; i <= n; i++) {
        x = x*i;
    }
    return x;
}

double combination(int n,int r) /* (nCr: n=e->degree,r=l) */
{
    double q;
    q=factorial(n)*pow(factorial(r),-1)*pow(factorial(n-r),-1);
    return q;
}

double n_fading_epsilon(double snr,double R){
    double eps=0;
    double theta = function_theta(snr, R);
    eps=1-exp((-1*theta) / (sigma_h2*snr));
    return eps;
}

double Decryption(int t,int r,Slotted_ALOHA& a){
    double dec=1.0;
        dec=pow((1+a.theta),-t*(r-(t+1)*0.5))*exp(-1*(pow((a.theta+1),t)-1)/(sigma_h2*a.snr));
        return dec;
}

double n_Ncollision_fading_eps(double snr,Slotted_ALOHA& a,int m){
    double eps=0.0;
    double sum=0.0;
    double R=m+1;
    double D=0;
    double dec=0;
    double K=0;

    for(int t=1;t<=R;t++){
        D=Decryption(t,R,a);
        dec+=factorial(R-1)/factorial(R-t)*D;
    }
    eps=1-dec;
//    eps=0; /* Multi-packet reception (MPR) channels */
    return eps;
}


void set_epsilon_n(Slotted_ALOHA& a)
{
 
  for(int i=0; i<=M;i++)
   {
       a.epsilon_n[i] =n_Ncollision_fading_eps(a.snr, a, i);
   }
}

void change_dist(Dist& Nd,Dist& Eh,Dist& Nh){
    double sum=0;
    double lsum=0;
    int l_degree;
    double l_coef;
    cout << "Edge distribution of all segment " << endl;
    for(vector<Edge>::iterator p = Nd.lambda.begin(); p!=Nd.lambda.end(); p++)
      {
          sum += (p->coef)*(p->degree+IS);
      }
    for(vector<Edge>::iterator p = Nd.lambda.begin(); p!=Nd.lambda.end(); p++)
      {
          l_coef=(p->coef)*(p->degree+IS)*pow(sum,-1);
          l_degree=p->degree+IS-1;
          Eh.add_Ledge(l_degree, l_coef);
          cout << l_degree << " " << l_coef << endl;
          lsum+=(l_coef/(l_degree+1));
      }
    cout << "Node distribution of all segment " << endl;
    for(vector<Edge>::iterator p = Eh.lambda.begin(); p!=Eh.lambda.end(); p++)
      {
        Nh.add_Ledge(p->degree+1, (p->coef/(p->degree+1))/lsum);
        cout << p->degree+1 << " " << (p->coef/(p->degree+1))/lsum << endl;
      }
}

double func_lambda(Dist& d, double x)
{
  double lam = 0.0,com,sum;
  for(vector<Edge>::iterator e = d.lambda.begin(); e != d.lambda.end(); e++)
    {
        sum=0;
        for(int l=0;l<=IS-1;l++){
            com=combination(e->degree,l);
            sum+=com*pow(1-x,l)*pow(x,e->degree-l);
        }
        lam += e->coef*sum;
    }
  return lam; 
}

double func_rho(double R, double G, double x)
{
    double dl = 0;
  return exp(-1*G*pow(R,-1)*(1-x));
}

double sum_epsilon(double R, double x, int m, Slotted_ALOHA& a)
{
  double sum = 0;
  double dl = 0;
  for(int i=0; i<=m;i++)
    {
      sum += ( (1-a.epsilon_n[i])/factorial(i))*pow(a.G*pow(R,-1)*x, i);
    }
  return sum;
}


void density_evolution_fading(Dist& Ed, Dist& Nd, int m, Slotted_ALOHA& a)
{
  double empty=0;
  double Gh = (double)(m+1);
  double Gl = 0.0;
  double q = 0;
  double R;
  set_epsilon_n(a);
  cout << "Parameter: ";
    double dl = 0;
    for(vector<Edge>::iterator e = Nd.lambda.begin(); e != Nd.lambda.end();e++)
      {
        dl+=e->coef*(e->degree);
      }
    R=IS*pow(dl,-1);
  cout << "SNR " << a.snr << ", M_Imax " << M << ", d " << dl <<", R " << R<< endl;
  for(int i=0; i<=m;i++) cout << "epsilon " << i << " - " << a.epsilon_n[i] << endl;
  for(int i=0; ;i++)
    {
      a.G = (Gh+Gl)/2;
      a.q=1;
      a.p=1;
      q=1;
      empty = 0;
      for(int j=0; ;j++)
        {
            empty = func_lambda(Nd,a.q);
          a.q = 1-func_rho(R, a.G, 1-func_lambda(Ed,a.q))*sum_epsilon(R, func_lambda(Ed,a.q), m, a );
            q = func_lambda(Nd,a.q);
      
          if(fabs(q - empty) <=  pow(10, -7)) break;
        }

      if(fabs(Gh - Gl) <= pow(10, -7)) break;
       
      if(q <= pow(10, -2)) {
        Gl = a.G;
      }
      else {
        Gh = a.G;
      }
    }
  cout << "G " << a.G << endl;
}

void file_input(Dist& E_dist, Dist& N_dist, Dist& Eh_dist, Dist& Nh_dist,char c[])
{
  ifstream inputfile(c);
  if(!inputfile)
    {
      cout << "Failed to read the file" << endl;
    }
  cout << "Input file is " << c << endl;
  int DL;
  int DR;
  double lsum=0;
  double rsum=0;
  inputfile >> DL;
  cout<<DL<<endl;
  vector<int> l_degree(DL);
  vector<double> l_coef(DL);
    cout << "Edge distribution of Parity segment " <<endl;
  for(int i=0; i<DL; i++)
    {
      inputfile >> l_degree[i] >> l_coef[i];
      E_dist.add_Ledge(l_degree[i], l_coef[i]);
      cout << l_degree[i] << " " << l_coef[i] << endl;
      lsum+=(l_coef[i]/(l_degree[i]+1));
    }
    cout << "Node distribution of Parity segment " << endl;
  for(int i=0; i<DL; i++)
    {
      N_dist.add_Ledge(l_degree[i]+1, (l_coef[i]/(l_degree[i]+1))/lsum);
      cout << l_degree[i]+1 << " " << (l_coef[i]/(l_degree[i]+1))/lsum << endl;
    }
   
    change_dist(N_dist,Eh_dist,Nh_dist);
  inputfile.close();
}


int main(int argc, char* argv[])
{ 
  Dist Ed;
  Dist Nd;
  Dist Eh;
  Dist Nh;
  Slotted_ALOHA a;

  file_input(Ed, Nd, Eh, Nh,argv[1]);

//  density_evolution_fading(Eh, Nh, 0, a); /* awgn */

  density_evolution_fading(Eh, Nh, M, a); /* fading */

  return 0;
  
}
