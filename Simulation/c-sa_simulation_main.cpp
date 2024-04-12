#include "sa-decoding.h"

/* Generate Random */
int random_edge(int slot, RandomNumberGenerator& seed)
{
  return seed.get_random_number(slot-1);
}

/* structure slot & device edge */
void structure_edge(Frame& str_frame, vector<Device>& f_device, RandomNumberGenerator& seed1, RandomNumberGenerator& seed2, Dist& dist, Channel& ch)
{
  int t;
  int count=0;
  int k=0;
  int times=0;
  list<int> li;
  list<double> f;
  
  for(int j=0; j<str_frame.device; j++) /*str_frame.device=Number of devices */
    {
      double eta = seed1.get_real_number();
      double under=0;
      double top=0;
      for(vector<DistEdge>::iterator l = dist.lambda.begin(); l != dist.lambda.end(); l++)
        {
          top+=l->coef;
          if(eta >= under && eta <= top) times = l->degree;
          under+=l->coef;
        }
        
        
      for(int i=0; i<times+K; i++)
        {
          k=seed1.get_random_number(str_frame.time_slot-1); /*(str_frame.time_slot) is the number of segments */
          if(i>0)
            {
              bool flag = false; 
              for(list<int>::iterator it=li.begin(); it!=li.end(); it++)
                {
                  if(*it == k) flag = true;
                }
              if(flag==true)
                {
                  continue;
                }
            }
          li.push_back(k);
            double fc = seed2.exponential_number((double)sigma_h2*1/(pow(10,-1*((double)SNR/10))));
          f.push_back(fc);
        }
      list<double>::iterator fr=f.begin();
      for (list<int>::iterator it=li.begin(); it!=li.end(); it++ )
        {
          f_device[j].add_edge(*it, *fr);
          str_frame.f_slot[*it].add_edge(j, *fr);
          fr++;
        }
      li.clear();
      f.clear();
    }

}

/* structure frame */
void structure_frame(Frame& str_frame, double g)
{
  int n_slot=0;
  int n_device=0;
    
  n_device = g*number_slot;
  str_frame.add_time_slot(number_segment);
  str_frame.add_device(n_device);
  for(int j=0; j<number_segment; j++) str_frame.add_slot();
}

void display(Frame& sc_frame, vector<Device>& f_device, double g)
{
  /* device */
  cout << "device" << '\n';
  for(int i=0; i<number_slot*g; i++)
    {
      list<Edge>::iterator d;
      for(d = f_device[i].d_edge.begin(); d != f_device[i].d_edge.end(); d++)
        {
          cout << "device(" << i << ")"<<  " - " <<  "segment:" << d->position << '\n';
        }
    }

  /* slot */
  cout << "segment" << '\n';

  int count = 0;
  for(int j=0; j<sc_frame.time_slot; j++)
    {
      list<Edge>::iterator e;
      for(e = sc_frame.f_slot[j].s_edge.begin(); e != sc_frame.f_slot[j].s_edge.end(); e++)
        {
          cout << "segment(" << count << ")" << " - " << "device:" << e->position <<  '\n';
        }
      count++;
    }
    
  cout << '\n';

}

/* 1 frame */
void frame(Frame& sc_frame, vector<Device>& f_device, double g, RandomNumberGenerator& seed1, RandomNumberGenerator& seed2, Dist& dist, Channel& ch)
{

  structure_frame(sc_frame, g);

  structure_edge(sc_frame, f_device, seed1, seed2, dist, ch);

/* display(sc_frame, f_device, g); */
  
}

void cancellation_slot(int device, int slot, Frame& sc_frame)
{
  int device_number = device;
  int slot_number = slot;

  for(list<Edge>::iterator e = sc_frame.f_slot[slot_number].s_edge.begin(); e != sc_frame.f_slot[slot_number].s_edge.end(); e++)
    {
      if(e->position == device_number)
        {
          e = sc_frame.f_slot[slot_number].s_edge.erase(e);
        }
    }
  
}


void successive_interference_cancellation(Frame& sc_frame, vector<Device>& f_device, int d, RandomNumberGenerator& seed, Channel& ch)
{
  int frame_number;	/* number of frame have slot connected single edge */
  int device_number;	/* number of slot connected single edge */
  int slot_number;
  int count1=0;
  int count2=0;
  for(int ic=0;; ic++)
    {

      count2=count1;

      for(int j=0; j<sc_frame.time_slot; j++)
        {
          int e_size = sc_frame.f_slot[j].s_edge.size();
           
          if(e_size <= M+1 && e_size > 0) 
            {
              for(int m=e_size; m>0; m--)
                {
                  for(list<Edge>::iterator ds = sc_frame.f_slot[j].s_edge.begin();ds != sc_frame.f_slot[j].s_edge.end();ds++)
                    {
                      double sinr = 0;
                        sinr = ds->snr;
                      double ir = 1;
                    
                      if(sc_frame.f_slot[j].s_edge.size() > 1)
                        {
                          for(list<Edge>::iterator is = sc_frame.f_slot[j].s_edge.begin(); is != sc_frame.f_slot[j].s_edge.end();is++)
                            {
                              if(is != ds)
                                {
                                    ir += is->snr;
                                }
                            } 
                        }
                      ch.snr = sinr*pow(ir,-1);
                        
                      /* success decode */
                      if(ch.set_epsilon() <1)
                        {
                          count1++;
                          device_number = ds->position;
                          slot_number=j;
                          cancellation_slot(device_number,slot_number, sc_frame); /* intra-sic */
                          f_device[device_number].counter++;
                            
                          if((f_device[device_number].d_edge.size() > 0)&&(f_device[device_number].counter>=K)) /*inter-sic */
                            {
                              for(list<Edge>::iterator qq = f_device[device_number].d_edge.begin(); qq != f_device[device_number].d_edge.end(); qq++)
                                {  
                                    cancellation_slot(device_number,qq->position, sc_frame);
                                }
                                  f_device[device_number].d_edge.clear();
                            }
                          break;
                        }
                    }
                }
            }
        }

      if((count1-count2) <= 0) break;

    }
}


double per(Frame& sc_frame, vector<Device>& f_device, int d)
{
  double error=0;

  for(int i=0; i<d; i++)
    {
      if(f_device[i].d_edge.size() > 0)
        {
          error++;
        }
    }
 return error*pow(d,-1);
}


void file_input(Dist& dist,char c[])
{

  ifstream inputfile(c);
  if(!inputfile)
    {
      cout << "Failed to read the file" << '\n';
    }
  int DL;
  int DR;
  double lsum=0;
  double rsum=0;
  double rate=0;
  inputfile >> DL;
  vector<int> l_degree(DL);
  vector<double> l_coef(DL);
    
  cout<<"degree distribution of edge-perspective"<< '\n';
  for(int i=0; i<DL; i++)
    {
      inputfile >> l_degree[i] >> l_coef[i];
      cout << l_degree[i] << " " << l_coef[i] << endl;
      lsum+=(l_coef[i]/(l_degree[i]+1));
      rate+=l_degree[i]*l_coef[i];
    }
  cout<< '\n';
  cout<<"degree distribution of node-perspective"<< '\n';
  for(int i=0; i<DL; i++)
    {
      cout << l_degree[i]+1 << " " << (l_coef[i]/(l_degree[i]+1))/lsum << '\n';
      dist.add_Ledge(l_degree[i]+1, (l_coef[i]/(l_degree[i]+1))/lsum);
    }
  cout<< '\n';
  inputfile.close();
  
}

void file_output(vector<double>& all_error, vector<double>& loss_trans, Channel& ch, char c[])
{

  ofstream outputfile(c);
  outputfile << "SNR " << SNR << "dB"<< '\n';
  outputfile << "code rate " << ch.R << '\n';
  outputfile << "slot " << number_slot << '\n';
  outputfile << "G PLR " << '\n';
  vector<double>::iterator b = loss_trans.begin();
  for(vector<double>::iterator a = all_error.begin(); a != all_error.end();a++)
    {
      outputfile << *b << " " <<  *a << '\n';
      b++;
    }
    
  outputfile.close();

}

 
int main(int argc, char* argv[])
{
  number_slot = atoi(argv[1]);
  number_segment = number_slot*K;
  Dist dist;
  file_input(dist, argv[2]);
  
  
  double packet_error_ratio = 0;
  vector<double> all_error;
  double trans=3.3;
  vector<double> loss_trans;
  int device = 0;
  RandomNumberGenerator seed1;
  RandomNumberGenerator seed2;
  RandomNumberGenerator s;
  Channel ch;
    cout << "G " << "PLR " << "Throughput " << '\n';
  for(;;)
    {
      packet_error_ratio = 0;
      cout << trans << " ";
      device = trans*number_slot;
      for(int i=0; i<ite; i++)
        {
          Frame sc_frame;
          vector<Device> f_device(device);

          frame(sc_frame, f_device, trans, seed1, seed2, dist, ch);
            /* display(sc_frame,f_device,trans); */
            
          successive_interference_cancellation(sc_frame, f_device, device, s, ch);

          packet_error_ratio += per(sc_frame, f_device, device);

        }
      all_error.push_back(packet_error_ratio/ite);
      loss_trans.push_back(trans);
      cout << packet_error_ratio/ite << " "<<trans*(1-packet_error_ratio/ite)<< '\n';
      s.reset_seed();

      if(trans <= 2.5) break;
      trans-=0.05;
    }
 

  file_output(all_error, loss_trans, ch, argv[3]);

  return 0;
    
}
