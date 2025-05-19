#include "lib.h"
#include "types.h"



void producer();
void consumer();
void test_producer_consumer();

int uEntry(void)
{
	// int ret = 0;


	// // For lab4.1
	// // Test 'scanf'
	// int dec = 0;
	// int hex = 0;
	// char str[6];
	// char cha = 0;
	// while (1) {
	// 	printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");

	// 	ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
	// 	// ret = scanf("test %c test %6s %d %x", &cha, str, &dec, &hex);
	// 	printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
	// 	// 输入：" Test a Test oslab 2025 0xabc"
	// 	if (ret == 4)
	// 		break;
	
	// }

	// For lab4.2
	// Test 'Semaphore'
	int i = 4;

	sem_t sem;
	printf("Parent Process: Semaphore Initializing.\n");
	ret = sem_init(&sem, 2);
	if (ret == -1) {
		printf("Parent Process: Semaphore Initializing Failed.\n");
		exit();
	}

	ret = fork();
	if (ret == 0) {
		while (i != 0)
		{
			i--;
			printf("Child Process: Semaphore Waiting.\n");
			sem_wait(&sem);
			// printf(".. Wait Over: ret = %d\n", ret);
			printf("Child Process: In Critical Area.\n");
		}
		printf("Child Process: Semaphore Destroying.\n");
		ret = sem_destroy(&sem);
		// printf(".. Destroy ret = %d\n", ret);
		exit();
	} else if (ret != -1) {
		while (i != 0)
		{
			i--;
			printf("Parent Process: Sleeping.\n");
			sleep(128);
			printf("Parent Process: Semaphore Posting.\n");
			ret = sem_post(&sem);
			// printf(".. Post ret = %d\n", ret);
		}
		printf("Parent Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		// exit();
	}

	// // For lab4.3
	// // Test 'Shared Variable'
	// printf("==============TEST SHAREDVARIABLE=============\n");
	// int number = 114514;

	// sharedvar_t svar;
	// ret = createSharedVariable(&svar, number);
	// printf("Parent Process: create Shared Variable: %d  with value: %d\n", svar, number);
	// if (ret == -1) exit();

	// ret = fork();
	// if (ret == 0) {
	// 	number = readSharedVariable(&svar);
	// 	printf("Child Process: readShared Variable: %d get value: %d\n", svar, number);
	// 	sleep(128);

	// 	number = readSharedVariable(&svar);
	// 	printf("Child Process: readShared Variable: %d get value: %d\n", svar, number);
	// 	number = 2333;
	// 	writeSharedVariable(&svar, number);
	// 	printf("Child Process: writeShared Variable: %d with value: %d\n", svar, number);

	// 	exit();
	// } else if (ret != -1) {
	// 	number = -5678;
	// 	sleep(64);

	// 	writeSharedVariable(&svar, number);
	// 	printf("Parent Process: writeShared Variable: %d with value: %d\n", svar, number);
	// 	sleep(128);

	// 	number = readSharedVariable(&svar);
	// 	printf("Parent Process: readShared Variable: %d get value: %d\n", svar, number);
	// 	sleep(128);

	// 	destroySharedVariable(&svar);
	// 	printf("Parent Process: destroyShared Variable: %d\n", svar);
	// 	exit();
	// }

	// For lab4.4
	// TODO: You need to design and test the problem.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.

	printf("==============TEST producer_consumer=============\n");
	test_producer_consumer();

	return 0;
}

#define PRODUCT_NUM 4
#define BUFFER_SIZE 4
#define SLEEP_TIME 128

sem_t mutex, empty, full;
int buffer[BUFFER_SIZE];
int in = 0, out = 0;

int product_id = 0;
int total_produced = 0;

void test_producer_consumer(){
	sem_init(&mutex, 1);
	sem_init(&empty, BUFFER_SIZE);
	sem_init(&full, 0);
	int ret;
	for (int i = 0; i < 4; i++) {
		ret = fork();  // 传入 id
		if(ret == 0){
			// printf("create a new producer thread\n");
			producer();
			exit();
		}
		// sleep(1000);
	}
	// printf("create a new consumer thread\n");
	consumer();
	
	sem_destroy(&mutex);
	sem_destroy(&empty);
	sem_destroy(&full);

	printf("==============TEST producer_consumer END=============\n");
}

void producer() {
	int pid = getpid();
	while (total_produced <= PRODUCT_NUM) {

		sleep(SLEEP_TIME);
		sem_wait(&mutex);
		
		sleep(SLEEP_TIME);
		sem_wait(&empty);

		sleep(SLEEP_TIME);

		// 生产
		// printf("Producer %d: produce: %d to buffer[%d]\n", pid - 1, product_id, in);
		printf("Producer %d: produce\n", pid - 1);
		buffer[in] = product_id;
		in = (in + 1) % BUFFER_SIZE;
		product_id++;
		total_produced++;

		sleep(SLEEP_TIME);
		sem_post(&mutex);

		sleep(SLEEP_TIME);
		sem_post(&full);
	}
}

void consumer() {
	int total_consumed = 0;
	while (total_consumed <= PRODUCT_NUM) {
		sleep(SLEEP_TIME);
		sem_wait(&mutex);
		
		sleep(SLEEP_TIME);
		sem_wait(&full);

		// 消费动作
		sleep(SLEEP_TIME);
		int item = buffer[out];
		printf("Consumer : consume\n");
		out = (out + 1) % BUFFER_SIZE;

		sleep(SLEEP_TIME);
		sem_post(&mutex);
		sleep(SLEEP_TIME);
		sem_post(&empty);
	}
}

