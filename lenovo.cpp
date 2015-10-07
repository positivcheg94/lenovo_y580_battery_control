#include <stdint.h>
#include <sys/io.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// EC table
#define EC_SC 0x66
#define EC_DATA 0x62

// control signals
#define IBF 1
#define OBF 0
#define EC_SC_READ_CMD 0x80
#define EC_SC_WRITE_CMD 0x81

// EC battery control cell
#define BATT_PORT 0x0a

// values of battery limit ( depends on laptop model )
#define BATT_LIMIT 0x20
#define BATT_FULL 0x40

static void init() {
	if (ioperm(EC_SC, 1, 1) != 0) {
		perror("ioperm(EC_SC, 1, 1)");
		exit(1);
	}
	if (ioperm(EC_DATA, 1, 1) != 0) {
		perror("ioperm(EC_DATA, 1, 1)");
		exit(1);
	}
}

static void wait_ec(const uint32_t port, const uint32_t flag, const char value)
{
	uint8_t data;
	int i;

	i = 0;
	data = inb(port);

	while ((((data >> flag) & 0x1) != value) && (i++ < 100))
	{
		usleep(1000);
		data = inb(port);
	}

	if (i >= 100)
	{
		fprintf(stderr,"wait_ec error on port 0x%x, data=0x%x, flag=0x%x, value=0x%x\n",port,data,flag,value);
		exit(1);
	}
}

static uint8_t read_ec(const uint32_t port)
{
	uint8_t value;

	wait_ec(EC_SC, IBF, 0);
	outb(EC_SC_READ_CMD, EC_SC);
	wait_ec(EC_SC, IBF, 0);
	outb(port, EC_DATA);
	wait_ec(EC_SC, OBF, 1);
	value = inb(EC_DATA);

	return value;
}

static void write_ec(const uint32_t port, const uint8_t value)
{
	wait_ec(EC_SC, IBF, 0);
	outb(EC_SC_WRITE_CMD, EC_SC);
	wait_ec(EC_SC, IBF, 0);
	outb(port, EC_DATA);
	wait_ec(EC_SC, IBF, 0);
	outb(value, EC_DATA);
	wait_ec(EC_SC, IBF, 0);
}

static void set_value(const uint8_t value)
{
	uint8_t rval;

	rval = read_ec(BATT_PORT);
	printf("old value %02x\n", rval);
	write_ec(BATT_PORT, value);
	rval = read_ec(BATT_PORT);
	printf("new value %02x\n", rval);
}

int main(int argc, char *argv[])
{
	init();
	if (argc < 2)
		printf("current value %02x\n", read_ec(BATT_PORT));
	else
	{
		if (strcmp(argv[1], "full") == 0) {
			printf("set full charge\n");
			set_value(BATT_FULL);
		}
		if (strcmp(argv[1], "limit") == 0) {
			printf("set limited charge\n");
			set_value(BATT_LIMIT);
		}
	}

	return 0;
}
