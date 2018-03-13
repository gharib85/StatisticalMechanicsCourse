#include <iostream>
#include <cmath>
#include <cstdio>
#include <fstream>

using namespace std;

const int N = 10;				//���Ӹ���
const int DIM = 2;				//����ά��
const int MAX_STEP = 10000000;	//����������
const double Ms = 0.2;			//Nose-Hover�ȿ����
const double Temperature =1.0;	//Nose-Hover�ȿ��¶�
const bool isNoseHover = true;	//Nose-Hover�ȿ⿪��

const double PI = 3.1415926;

double* pos;				//�洢λ��
double*	pre_pos;			//�洢��һ��λ��
double* velocity;			//�洢�ٶȣ���δʹ�ã�
double* force;				//�洢����
double* potential;
double* kinetic_energy;
double max_force;			//�洢�������
double eta = -1.0;
double time_step;	//ʱ�䲽��
double time=0.0;	//ͳ�Ƶ�ǰ����ʱ��
int step;

void initialize(){
	//�˴����ó�ʼλ��
	//��ʼλ�ð���Բ�ܾ��ȷֲ�
	for (int i = 0; i < 10; i++) {
		pos[i*DIM + 0] = cos(2*PI * ((double)i) / N);
		pos[i*DIM + 1] = sin(2*PI * ((double)i) / N);
	}

	for(int i=0;i<DIM*N; i++)
		pre_pos[i] = pos[i];	

	for(int i=0;i<DIM*N; i++)
		*(velocity+i) = 0.0;
}

void compute_force(){
	/*
	  totalForce ��������ij֮����໥����������,����Ϊ��
	  dist ��������֮��ľ���
	  */
	for(int i=0; i<N*DIM; i++){
		force[i] = 0.0;				//��ÿ�����ӵ���������
		max_force = 0.0;			//�洢�����������ڱ䲽�����㣩
	}
	double  r, x, y, dist, dx, dy, totalForce;
	for(int i=0;i<N;i++){
		x = pos[i*DIM + 0];
		y = pos[i*DIM + 1];
		r = x*x + y*y;
		force[i*DIM + 0] += -x;		//����������
		force[i*DIM + 1] += -y;		//����������
		for(int j=i+1;j<N;j++){
			dx = pos[i*DIM + 0] - pos[j*DIM + 0];
			dy = pos[i*DIM + 1] - pos[j*DIM + 1];
			dist = sqrt(dx*dx+dy*dy);
			totalForce = (48*pow(dist,-13.0)-24*pow(dist,-7.0));			
			//printf("i=%d,j=%d, dist ij = %f, force = %f\n",i,j,dist, totalForce);
			force[i*DIM + 0] += totalForce*dx/dist;
			force[i*DIM + 1] += totalForce*dy/dist;
			force[j*DIM + 0] -= totalForce*dx/dist;
			force[j*DIM + 1] -= totalForce*dy/dist;
		}
		if(isNoseHover){				//��Nose-Hover�ȿ�
			//eta = 1.0;
			force[i*DIM + 0] += -eta*velocity[i*DIM + 0];
			force[i*DIM + 1] += -eta*velocity[i*DIM + 1];
			//if(eta > 50 ) printf("step=%d,time=%f,fx=%f,fy=%f,x=%6f,y=%6f,vx=%f,vy=%f,eta=%f\n",
			//	step,time,force[i*DIM+0],force[i*DIM+1],pos[i*DIM],pos[i*DIM+1],velocity[i*DIM+0],velocity[i*DIM+1],eta);
		}
		//if(abs(force[i*DIM + 0]>max_force)) max_force = abs(force[i*DIM + 0]);
		//if(abs(force[i*DIM + 1]>max_force)) max_force = abs(force[i*DIM + 1]);
	}
	
	for(int i=0; i<N; i++){
		//printf("i=%d,fx=%f,fy=%f,vx=%f,vy=%f\n",i,force[i*DIM+0],force[i*DIM+1],velocity[i*DIM+0],velocity[i*DIM+1]);
	}
}

// Verlet �㷨
void compute_pos(double step = 1e-5){
	for(int i=0;i<N*DIM;i++){
		pre_pos[i] = 2*pos[i]-pre_pos[i]+step*step*force[i];  // Verlet �㷨
		velocity[i] = - (pos[i]-pre_pos[i]) / step;
	}
	swap(pre_pos,pos);
}

void compute_energy(){
	double  dist, dx, dy;
	for(int i=0;i<N;i++){
		kinetic_energy[i] = 0.5*velocity[i*DIM+0]*velocity[i*DIM+0]
							+0.5*velocity[i*DIM+1]*velocity[i*DIM+1];
		potential[i] = 0.5*(pos[i*DIM+0]*pos[i*DIM+0]+pos[i*DIM+1]*pos[i*DIM+1]);
		for(int j=0;j<N;j++){
			if (i==j) continue;
			dx = pos[i*DIM + 0] - pos[j*DIM + 0];
			dy = pos[i*DIM + 1] - pos[j*DIM + 1];
			dist = sqrt(dx*dx+dy*dy);
			potential[i] += 4*(pow(dist,-12)-pow(dist,-6));
		}
	}
}

//��ӡ�������ӵ�λ����Ϣ
void print_pos(int step = 0){
	for(int i=0; i<N; i++){
		printf("step:%d,i=%d,x=%f,y=%f\n",step,i,pos[i*DIM+0],pos[i*DIM+1]);
	}
	cout << endl;
}

//Nose-Hover �ȿ�
//a = f/m -> a=f/m - v*eta
//�˲��ּ���eta
void compute_eta(double timeStep){
	double total_velocity_square = 0.0;
	for(int i=0; i<N; i++){
		total_velocity_square += velocity[i*DIM+0]*velocity[i*DIM+0]
								+velocity[i*DIM+1]*velocity[i*DIM+1];
	}
	eta += timeStep*(total_velocity_square - N*DIM*Temperature)/Ms;	//�˴������ʣ���ʽ�е�h�������ɶȣ�
}

int main(){
	pos			= new double[N*DIM];
	pre_pos		= new double[N*DIM];
	velocity	= new double[N*DIM];
	force		= new double[N*DIM];
	potential   = new double[N];
	kinetic_energy = new double[N];
	initialize();
	print_pos(0);
	system("pause");
	ofstream out("result.txt");

	for(step=0;step<MAX_STEP;step++){
		time_step = 1e-5;//1.0/max_force;		//����Ӧ��ʹ�õ�ʱ�䲽����ȡΪ�̶�������
		if(isNoseHover){
			compute_eta(time_step);				//�����ȿ�Ħ����
		}
		compute_force();					//����������������		
		compute_pos(time_step);				//��ǰ�ƽ�һ��
		time+=time_step;					//����˲�����ʱ��
		if(step%10000 == 0)					//ÿ��10000���洢һ������
		{			
			compute_energy();
			out << time << " " ;
			for(int in=0; in<N; in++){
				out << pos[in*DIM +0] << " " << pos[in*DIM +1] << " "
					<< velocity[in*DIM +0] << " " << velocity[in*DIM +1] << " "
					<< kinetic_energy[in] << " " << potential[in] << " ";
			}
			out << endl;
			if(step%1000000 == 0)				//ÿ��1000000��ʾһ�ν���
				print_pos(step);
		}
	}
	out.close();
	return 0;
}

