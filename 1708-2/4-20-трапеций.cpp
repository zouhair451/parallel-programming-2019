#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

double y(double x);    /* Интегрируемая функция */
void Trap(double a, double b, int n, double* global_result_p);

int main(int argc, char* argv[]) {
	double  global_result = 0.0;  /* Хранить результат в global_result */
	double  a, b;                 /* Левый и правый концы отрезка  */
	int     n;                    /* Количество трапеций           */
	int     thread_count;

	// Количество потоков:
	thread_count = 10;

	a = 0;
	b = 5;
	n = (b - a) / 0.00000005;

	if (n % thread_count != 0) {
		fprintf(stderr, "Количество трапеций должно ровно делиться на количество потоков\n");
		exit(0);
	}

	// Директива parallel создает параллельный регион для следующего за ней
	// структурированного блока, например:
	//  #pragma omp parallel [раздел[ [,] раздел]...]
	//  структурированный блок
	// Эта директива сообщает компилятору, что структурированный блок кода должен
	// быть выполнен параллельно, в нескольких потоках. Каждый поток будет
	// выполнять один и тот же поток команд, но не один и тот же набор команд -
	// все зависит от операторов, управляющих логикой программы,
	// таких как if-else.
#pragma omp parallel num_threads(thread_count)
	Trap(a, b, n, &global_result);

	printf("С n = %d трапеций, приближенное значение\n", n);
	printf("определенного интеграла от %f до %f = %.14e\n",
		a, b, global_result);
	getchar();
	return 0;
}  /* main */

/*------------------------------------------------------------------
* Функция:                y
* Назначение:             Посчитать значение интегрируемой функции
* Входной аргумент:       x
* Возвращаемое значение:  y(x)
*/
double y(double x) {
	double return_val;

	return_val = exp(x) * x - cos(x);
	return return_val;
}  /* y */

/*------------------------------------------------------------------
* Функция:    Trap
* Назначение:     Метод трапеций
* Входные аргументы:
*    a: левый конец отрезка
*    b: правый конец отрезка
*    n: количество трапеций
* Выходные аргументы:
*    integral:  приближенное значение определенного интеграла от a до b для y(x)
*/
void Trap(double a, double b, int n, double* global_result_p) {
	double  h, x, my_result;
	double  local_a, local_b;
	int  i, local_n;

	// Номер потока в группе потоков:
	// Число в диапазоне от 0 до omp_get_num_threads - 1.
	int my_rank = omp_get_thread_num();

	// Число потоков, входящих в текущую группу потоков:
	// Если вызывающий поток выполняется не в параллельном регионе,
	// эта функция возвращает 1.
	int thread_count = omp_get_num_threads();

	h = (b - a) / n;
	local_n = n / thread_count;
	local_a = a + my_rank*local_n*h;
	local_b = local_a + local_n*h;
	my_result = (y(local_a) + y(local_b)) / 2.0;
	for (i = 1; i <= local_n - 1; i++) {
		x = local_a + i*h;
		my_result += y(x);
	}
	my_result = my_result*h;

	// Использование критической секции в качестве барьера:
	// В OpenMP для входа в критическую секцию применяется директива
	//  #pragma omp critical [имя].
	// Доступ к блоку кода является взаимоисключающим только для других
	// критических секций с тем же именем (это справедливо для всего процесса).
	// Если имя не указано, директива ставится в соответствие некоему имени,
	// выбираемому системой. Доступ ко всем неименованным критическим секциям
	// является взаимоисключающим.
#pragma omp critical
	*global_result_p += my_result;

}  /* Trap */