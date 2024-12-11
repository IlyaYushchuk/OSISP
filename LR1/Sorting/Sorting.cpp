#include <iostream>
#include <string>
#include <vector>
#include <stdexcept> 

using namespace std;

int bubblesort(vector<int> mass, int n)
{
	int opCount = 0;
	for (int i = 1; i < n; ++i)
	{
		for (int r = 0; r < n - i; r++)
		{
			if (mass[r] < mass[r + 1])
			{
				// Обмен местами
				int temp = mass[r];
				mass[r] = mass[r + 1];
				mass[r + 1] = temp;
			}
			opCount++;
		}
	}
	return opCount;
}

int main(int argc, char* argv[])
{
	if (argc == 3)
	{
		throw logic_error("aa");
	}
	cout << "ARGC " << argc << endl;
	for (int i = 0; i < argc; i++)
	{
		cout << "ARGV" << i<<" "<<argv[i] << endl;

	}
	
	/* Установим размер массива */
	int n = stoi(argv[1]);; // Кол-во элементов
	

	/* Заполним массив значениями */
	vector<int> mass;
	mass.assign(n, 0);
	for (int i = 0; i < n; ++i)
	{
		//cout << i + 1 << "-ый элемент: ";
		mass[i] =i+1;
	}

	/* Выведем исходный массив */
	cout << "Array: ";
	for (int i = 0; i < n; ++i)
	{
		cout << mass[i] << " ";
	}
	cout << endl;
	
	int count = bubblesort(mass, n);

	cout << "Sorted array: ";
	for (int i = 0; i < n; ++i)
	{
		cout << mass[i] << " ";
	}
	cout << endl;


	return 0;
}

