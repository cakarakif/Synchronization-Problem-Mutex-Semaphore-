#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

//student'ların maksimum sayısı
#define MAX_STUDENTS 100
//driver ve carChair'ların maksimum sayısı
#define MAX_DRIVER 10
#define MAX_CAPACITY 4

//Taksinin koltuklarındaki yolcuları tutmak üzere struct yapısını kurduk.
struct taxiSeats {
   int numOne;
   int numTwo;
   int numThree;
   int numFour;
}taxiSeat;

//Fonksiyonların tanımlamaları
void *student(void *num);
void *driver(void *);
void randwait(int secs);

// waitingArea bir defada bekleme alanına girmesine izin verilen student sayısını sınırlar.(Ödevde bu sınırlamaya gerek yoktur sadece simgesel)
sem_t waitingArea;

//carChair, araç koltuğuna dört student'in oturabilmesini sağlar.
sem_t carChair;

//driverPillow bir student gelene kadar driver'ların uyumasına izin vermek için kullanılır.
sem_t driverPillow;

//seatBelt, driver'ların ulaştırma işlemi tamamlayana kadar student'i beklettirmek için kullanılır.
sem_t seatBelt;

//Tüm student'lar bölümlerine ulaştırıldığında driver'ları eve göndermek için kullanıyoruz.
int allDone = 0;
int waitingAreaValue=-1;
int temp=0;

int main(int argc, char *argv[]) {
    //driver thread id, tanımlama yaptık
    pthread_t btid[MAX_DRIVER];
    //students thread id, tanımlama yaptık
    pthread_t tid[MAX_STUDENTS];
	
    int i, numstudents, numStops, numDrivers;
    int Number[MAX_STUDENTS];
    int NumbercarChair [MAX_DRIVER];
    //İstenilen sayı sınırlamalarını kullanmak için değişkenlere attık.
    numstudents = 100;
    numStops = 100; //Durakta aynı anda bekleme kapasitesi(berber dükkanındaki koltuk mantığı)
    numDrivers = 10;
    waitingAreaValue=numStops;

    // Number dizisine student no atar.
    for (i=0; i<MAX_STUDENTS; i++) {
    Number[i] = i;
    }
    //NumbercarChair dizisine carChair no atar.
     for (i=0; i<MAX_DRIVER; i++) {
    NumbercarChair[i] = i;
    }
    
	//Başlangıç değerleriyle semaphorları başlattık.
    //numStops kadar alan içeren bekleme alanı
    sem_init(&waitingArea, 0, numStops);
    //her numDrivers MAX_CAPACITY kadar carChair içerir. 
    sem_init(&carChair, 0, MAX_CAPACITY);
    sem_init(&driverPillow, 0, 0);
    sem_init(&seatBelt, 0, 0);

    // driver'ları yarat.	
    for(i=0;i<numDrivers;i++){
    pthread_create(&btid[i], NULL, driver,(void *)&NumbercarChair[i]);
    }

    // students'ları yarat.
    for (i=0; i<numstudents; i++) {
		pthread_create(&tid[i], NULL, student, (void *)&Number[i]);
    }

    // student threadlerinin sırayla bitirilmesini garantilemek için join fonksiyonu kullandık.
    for (i=0; i<numstudents; i++) {
    pthread_join(tid[i],NULL);	
    }
	
    //Tüm student threadleri bittiğinde, driver threadini bitirdik. 
    allDone = 1;
	//sem_post fonk. ile driverPillow semaphorundaki kilit kaldırılır.
    waitingAreaValue=1; 
    for(i=0;i<numDrivers;i++){
		sem_post(&driverPillow);
	}
	for(i=0;i<numDrivers;i++){
		pthread_join(btid[i],NULL);    
    }
    printf("THE PROGRAM FINISHED.\n");
}

void *student(void *number) {
	int num = *(int *)number;

	printf("Student %d is leaving home to reach the stop.\n", num);
	randwait(2);
	printf("Student %d has arrived at the stop and has been waiting for the taxi.\n", num);
	
	
	// waitingArea semaphoru dört student için sem_wait fonk. ile kilitlendi.
	sem_wait(&waitingArea);
	waitingAreaValue-=1;
	// carChair semaphoru  bir student için sem_wait fonk. ile kilitlendi.
	sem_wait(&carChair);
	//student carChair'ı kilitledikten sonra, sem_post fonk. ile waitingArea semaphorun'daki kilit kaldırılıyor.
	sem_post(&waitingArea);
	waitingAreaValue+=1;
	int value;
	sem_getvalue(&carChair, &value);
	if(value==3)taxiSeat.numOne=num;
	else if(value==2)taxiSeat.numTwo=num;
	else if(value==1)taxiSeat.numThree=num;
	else taxiSeat.numFour=num;
	sem_post(&driverPillow);

	// Bölüme ulaştırma işlemi bitene kadar sem_wait fonksiyonu ile seatBelt semaphoru kilitleniyor.
	sem_wait(&seatBelt);
	printf("Student %d has arrived to the department.\n", num);
	
	// carChair semaphorundaki kilit kaldırılıyor.
	// diğer student'ler araca binebilir.
	sem_post(&carChair);
	randwait(3);
	if(value==0){
		printf("Taxi %d has finished transferring and returned to stop.\n", temp);
	    printf("Driver %d is sleeping.\n", temp);
	}
		
}

void *driver(void *junk) {
	int jun = *(int *)junk;
	//Araç içindeki kapasite dolana kadar takip eder
	int value; 
	//Hizmet edilecek müşteri olduğu sürece devam eder.
	printf("Driver %d is sleeping.\n", jun);
	while (!allDone) {
		//Biri varana kadar uyur ve varan kişi uyandırır.
		sem_wait(&driverPillow);
		
		if(!allDone){// driver uyandırılıyor.
			printf("Driver %d was awakened by Student %d.\n", jun,taxiSeat.numOne);
			value=1;
			while(value<MAX_CAPACITY){
				if(value==1)
					printf("[Taxi %d>>|%d|X|X|X|]",jun,taxiSeat.numOne);
				else if(value==2)
					printf("[Taxi %d>>|%d|%d|X|X|]",jun,taxiSeat.numOne,taxiSeat.numTwo);
				else if(value==3)
					printf("[Taxi %d>>|%d|%d|%d|X|]",jun,taxiSeat.numOne,taxiSeat.numTwo,taxiSeat.numThree);
				
				printf("The last %d students, let's get up!\n",(MAX_CAPACITY-value));
				value++;
				sem_wait(&driverPillow);
			}
		}
		
	
		// hizmet edilecek student kapasitesi dolduysa tranfer işlemi başlar.
		if (!allDone) {
			
			//Transfer işlemi için rastgele bir zaman harcanır.
			printf("[Taxi %d>>|%d|%d|%d|%d|]The transfer of the students begins...\n", jun,taxiSeat.numOne,taxiSeat.numTwo,taxiSeat.numThree,taxiSeat.numFour);
			randwait(1);
			temp=jun;
			// Bölümlerine ulaştırılan student'ler serbest bırakılır.
			int freeAll=MAX_CAPACITY;
			while(freeAll>0){
				sem_post(&seatBelt);
				freeAll--;
			}
			
		}
		//student yoksa driver evine gitsin.
		else {
			printf("Driver %d is going to the home for the day.\n", jun);
		}
	}

}

void randwait(int secs) {
     int len = 1; // Aralardaki bekleme süreleri
     sleep(len);
}
