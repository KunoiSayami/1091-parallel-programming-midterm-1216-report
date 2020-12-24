#include <cstdio>
#include <cmath>
#include <omp.h>
#include <ctime>
#include <sys/types.h>
#include <sys/timeb.h>
#include <cmath>
#include <cstring>


#ifdef VC2005
/* save time, in milli second unit, into ltime */
void times2(long *mtime)
{
	__time64_t ltime;
	struct __timeb64 tstruct;
	_time64(&ltime);
	_ftime64_s(&tstruct);
	*mtime = ltime * 1000 + tstruct.millitm;
}
#else

/* save time, in milli second unit, into ltime */
void times2(long *mtime) {
	time(mtime);
}

#endif

double utime() {
	static long t1, t2;
	static int first = 1;
	if (first) {
		times2(&t1);
		first = 0;
		return (0.0);
	} else {
		times2(&t2);
		first = 1;  // reset this counter
	}
	return ((t2 - t1) / 1000.0);
}


#define UTIME_LNG 1000

/* A function to return the used CPU time */
double utimeA(int idx, const char *str) {
	static long t1[UTIME_LNG], t2[UTIME_LNG];
	static int first[UTIME_LNG];
	static int initial = 1;
	double tval;
	int i;
	if (initial) {
		initial = 0;
		for (i = 0; i < UTIME_LNG; i++) first[i] = 1;
	}
	if (idx >= UTIME_LNG) {
		printf("\nWrong in giving index to utimeA().\n");
		exit(-1);
	}
	if (first[idx]) {
		times2(&t1[idx]);
		first[idx] = 0;
		return (0.0);
	} else {
		times2(&t2[idx]);
		first[idx] = 1;  // reset this counter
	}
	tval = (t2[idx] - t1[idx]) / 1000.0;
	printf("%s %g second.", str, tval);
	return (tval);
}

/*  ............... Testing programs ................. */

#define SORT_ORDER 0
#define EQ ==
constexpr size_t default_sz = 50000;
//WARNING: "if define this length more than 840, process will stuck" (fixed)
constexpr size_t LNG = default_sz - 1;
constexpr size_t STRING_SIZE = 20;
constexpr int MAX_NUM = 10000;
int printing = 0;

void printKey(char **key, int *t) {
	int i;
	printf("\n\tcontent:");
	for (i = 1; i <= LNG; i++) printf("%s  ", key[t[i]]);
}

int changed = 0;    // the global variable
int use_changed = 1;  // flag to decide whether go recording whether has exchanged
void comp_exchange2(char **key, int *t, int a, int b, int len) {
	int tmp;
	if (b > len) return;   // b is out of original_n, so key[t[a]] is smaller
	if (strcmp(key[t[a]], key[t[b]]) > 0) {
		// exchange
		tmp = t[a];
		t[a] = t[b];
		t[b] = tmp;
#pragma omp atomic // record the exchange counts
		changed++;
	}
}

void comp_exchange(char **key, int *t, int a, int b) {
	int tmp;
	if (strcmp(key[t[a]], key[t[b]]) > 0) {
		// exchange
		tmp = t[a];
		t[a] = t[b];
		t[b] = tmp;
#pragma omp atomic // record the exchange counts
		changed++;
	}
}

void trsort(int n, char **key, int *t, int para) {
	int i, j;
	int thN;
	changed = 0;
	for (i = 1; i <= n; i++) {
		//printf("\niteration %d", i);
		if (i % 2) {  // i is odd
#pragma omp parallel for default(none) shared(n, key, t), private(j) if(para)
			for (j = 0; j <= n / 2 - 1; j++) {
				comp_exchange(key, t, 2 * j + 1, 2 * j + 2);
			}
		} else {  // i is even
			// add following three lines to cope the problem of n is odd
			int m;
			if (n % 2) m = n / 2;
			else m = n / 2 - 1;
#pragma omp parallel for default(none) shared(m, t, key), private(j) if(para)
			for (j = 1; j <= m; j++) {
				comp_exchange(key, t, 2 * j, 2 * j + 1);
			}
			if (use_changed) {
				if (!changed) break;
				else changed = 0;
			}
		}
	}
}

void shsort(int n, char **key, int *t, int para) {
	int R, i, j, j2, pi, offset, pi2;
	int thN;
	int a, original_n;
	// length consideration
	for (a = 0; ; a++) if (pow(2.0, (double) a) >= n) break;
	original_n = n;
	n = pow(2.0, (double) a);
	R = n;
	while (R >= 2) {
		//#pragma omp parallel for private(i, pi, offset, pi2, j, j2) if(para) num_threads(8)
		for (i = 0; i < n / 2; i++) {
			pi = i % (R / 2);
			offset = (int) (i / (R / 2)) * R;
			pi2 = R - pi - 1;
			j = pi + offset;
			j2 = pi2 + offset;
#ifdef PRINT_PROCESS
			if (printing) printf("\ncomp %d %d ", j, j2);
#endif
			comp_exchange2(key, t, j + 1, j2 + 1, original_n);
		}
#ifdef PRINT_PROCESS
		if (printing) printf("\n----");
#endif
		R /= 2;
	}
	trsort(original_n, key, t, para);
}


int main(int argc, char const * argv[]) {
	bool read_from_file = false;
	if (argc > 1) {
		for (int i = 1; i < argc; i++)
			if (!strcmp(argv[i], "--file")) {
				freopen("data.in", "r", stdin);
				read_from_file = true;
			}
	}
	char **ar, tmp[default_sz];
	int t[default_sz];
	int maxNum = MAX_NUM;
	//omp_set_nested(1);
	/* set the content of ar array */
	ar = new char *[default_sz];
	//ar = (char **) calloc(default_sz, sizeof(char *));
	for (int i = 1; i <= LNG + 1; i++)
		ar[i] = new char[STRING_SIZE];
	//ar[i] = (char *) calloc(STRING_SIZE, 1);
	for (int i = 1, tmp; i <= LNG; i++) {
		printf("\rcurrent i: %d", i);
		if (read_from_file) {
			scanf("%d", &tmp);
		}
		else {
			tmp = rand() % maxNum + 1;
			while (tmp < maxNum / 10 - 1)
				tmp *= 10;
		}
		sprintf(ar[i], "%d", tmp);
	}
	for (int i = 1; i <= LNG; i++)
		t[i] = i;
	printf("\nBefor sort:");
	if (printing)
		for (int i = 1; i <= LNG; i++)
			printf("%s ", ar[t[i]]);

	// first sort
	utimeA(1, "");
	shsort(LNG, ar, t, 0);
	utimeA(1, "\nused time:");
	printf("\nfirst sort, shell sort sequential:");
	if (printing)
		for (int i = 1; i <= LNG; i++)
			printf("%s ", ar[t[i]]);

	// second sort
	for (int i = 1; i <= LNG; i++)
		t[i] = i;
	utimeA(1, "");
	shsort(LNG, ar, t, 1);
	utimeA(1, "\nused time:");
	printf("\nsecond sort, shell sort parallel:");
	if (printing)
		for (int i = 1; i <= LNG; i++)
			printf("%s ", ar[t[i]]);

	// third sort
	for (int i = 1; i <= LNG; i++) t[i] = i;
	utimeA(1, "");
	trsort(LNG, ar, t, 0);
	utimeA(1, "\nused time:");
	printf("\nthird sort, transposition sort parallel:");
	if (printing)
		for (int i = 1; i <= LNG; i++)
			printf("%s ", ar[t[i]]);
	//getchar();
	for (int i = 1; i <= LNG + 1; i++)
		delete [] ar[i];
	delete [] ar;
}
