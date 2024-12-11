#include <windows.h>
#include <iostream>


using namespace std;


const int NUM_PHILOSOPHERS = 6;
int iter = 100;
HANDLE mutex;  // Мьютекс для синхронизации
HANDLE forkControl; // Общий семафор для контроля захвата вилок
const DWORD TIMEOUT = 100;  // Таймаут ожидания вилки в миллисекундах

bool show = false;


int ATTITUDE_MODEL = 0;

// 0 - естественный
// 1 - классичческий
// 2 - запрет на действия пока 1 захватывает вилки
// 3 - запрет на захват пока 1 захватывает вилки
// 4 - избыточность
// 5 - тайм аут

int* NUMBER_OF_MEALS = new int[NUM_PHILOSOPHERS] {0};
HANDLE semaphores[NUM_PHILOSOPHERS];  // Семафоры для философов
int THINKING_TIME = 100;
int EATING_TIME = 2;
HANDLE* forks = new HANDLE[NUM_PHILOSOPHERS];  // Семафоры для вилок
HANDLE philosophers[NUM_PHILOSOPHERS];  // Потоки философов

DWORD WINAPI Philosopher(LPVOID lpParam)
{
    int id = (int)(intptr_t)lpParam;
    int leftFork = id;
    int rightFork = (id + 1) % NUM_PHILOSOPHERS;

    while (iter > 0) {
        // Философ думает
        if(show)
        {
            cout << "Философ " << id << " думает...\n";
        }
        Sleep(THINKING_TIME);

        
        if (ATTITUDE_MODEL == 0)
        {
                // Философ пытается взять вилки
            WaitForSingleObject(forks[leftFork], INFINITE);  // Захват левой вилки
            WaitForSingleObject(forks[rightFork], INFINITE); // Захват правой вилки
        }
        else if(ATTITUDE_MODEL == 1)
        {     // Попытка захватить семафор (право на еду)
            WaitForSingleObject(semaphores[id], INFINITE);  // Захват семафора философа
            if (show)
            {
                cout << "Философ " << id << " захватил :" << id << "\n";
            }

            if (id == 0) {
                // Философ 0: проверяем соседа 1 и 4
                WaitForSingleObject(semaphores[1], INFINITE);
                if (show)
                {
                    cout << "Философ " << id << " захватил :" << 1 << "\n";
                }
                WaitForSingleObject(semaphores[NUM_PHILOSOPHERS - 1], INFINITE);
                if (show)
                {
                    cout << "Философ " << id << " захватил :" << NUM_PHILOSOPHERS - 1 << "\n";
                }
            }
            else {
                // Другие философы: проверяем соседей
                WaitForSingleObject(semaphores[id - 1], INFINITE);

                if (show)
                {
                    cout << "Философ " << id << " захватил :" << id - 1 << "\n";
                }
                WaitForSingleObject(semaphores[(id + 1) % NUM_PHILOSOPHERS], INFINITE);
                if (show)
                {
                    cout << "Философ " << id << " захватил :" << (id + 1) % NUM_PHILOSOPHERS << "\n";
                }
            }
        }
        else if (ATTITUDE_MODEL == 2)
        {    
            WaitForSingleObject(mutex, INFINITE);  // Захват мьютекса

            //cout << "Философ " << id << " захватил мьютекс...\n";
            WaitForSingleObject(forks[leftFork], INFINITE);  // Захват левой вилки
            WaitForSingleObject(forks[rightFork], INFINITE); // Захват правой вилки
        }
        else if (ATTITUDE_MODEL == 3)
        {
            WaitForSingleObject(forkControl, INFINITE);  // Захват мьютекса
            WaitForSingleObject(forks[leftFork], INFINITE);  // Захват левой вилки
            WaitForSingleObject(forks[rightFork], INFINITE); // Захват правой вилки

            // Освобождение семафора для других философов
            ReleaseSemaphore(forkControl, 1, NULL);

        }
        else if (ATTITUDE_MODEL == 4)
        {
            // Философ пытается взять вилки
            WaitForSingleObject(forks[id*2], INFINITE);  // Захват левой вилки
            WaitForSingleObject(forks[id*2+1], INFINITE); // Захват правой вилки

        }
        else if (ATTITUDE_MODEL == 5)
        {
            DWORD result;
            // Захват левой вилки с таймаутом
            result = WaitForSingleObject(forks[leftFork], TIMEOUT);
            if (result == WAIT_OBJECT_0) {  // Успешно захвачена левая вилка
                // Попытка захватить правую вилку с таймаутом
                result = WaitForSingleObject(forks[rightFork], TIMEOUT);
                if (result == WAIT_OBJECT_0) {  // Успешно захвачена правая вилка
                    //// Философ ест
                    //cout << "Философ " << id << " ест...\n";
                    //mealCount[id]++;
                 
                }
                else {
                    // Не удалось захватить правую вилку, освобождаем левую
                    ReleaseSemaphore(forks[leftFork], 1, NULL);
                   // cout << "Философ " << id << " освободил левую вилку...\n";
                }
            }
        }

        // Философ ест
        if (show)
        {
            cout << "Философ " << id << " ест..." << "Iter " << iter << '\n';
        }
        NUMBER_OF_MEALS[id]++;
        Sleep(EATING_TIME);

        
        if (ATTITUDE_MODEL == 0)
        {
            // Освобождаем вилки
            ReleaseSemaphore(forks[leftFork], 1, NULL);
            ReleaseSemaphore(forks[rightFork], 1, NULL);

        }
        else if (ATTITUDE_MODEL == 1)
        {
            if (id == 0) {
                // Философ 0: проверяем соседа 1 и 4
                ReleaseSemaphore(semaphores[1], 1, NULL);

                if (show)
                {
                    cout << "Философ " << id << " отпустил :" << 1 << "\n";
                }
                ReleaseSemaphore(semaphores[NUM_PHILOSOPHERS - 1], 1, NULL);
                if (show)
                {
                    cout << "Философ " << id << " отпустил :" << NUM_PHILOSOPHERS - 1 << "\n";
                }
            }
            else {
                // Другие философы: проверяем соседей
                ReleaseSemaphore(semaphores[id - 1], 1, NULL);
                if (show)
                {
                    cout << "Философ " << id << " отпустил :" << id - 1 << "\n";
                }ReleaseSemaphore(semaphores[(id + 1) % NUM_PHILOSOPHERS], 1, NULL);
                if (show)
                {
                    cout << "Философ " << id << " отпустил :" << (id + 1) % NUM_PHILOSOPHERS << "\n";
                }
            }
            // Освобождение своего семафора
            ReleaseSemaphore(semaphores[id], 1, NULL);
            if (show)
            {
                cout << "Философ " << id << " отпустил :" << id << "\n";
            }
        }
        else if (ATTITUDE_MODEL == 2)
        {
            // Освобождение мьютекса
            ReleaseMutex(mutex);
            ReleaseSemaphore(forks[leftFork], 1, NULL);
            ReleaseSemaphore(forks[rightFork], 1, NULL);

        }
        else if (ATTITUDE_MODEL == 3)
        {
            ReleaseSemaphore(forks[leftFork], 1, NULL);
            ReleaseSemaphore(forks[rightFork], 1, NULL);

        }
        else if (ATTITUDE_MODEL == 4)
        {
            ReleaseSemaphore(forks[id * 2], 1, NULL);
            ReleaseSemaphore(forks[id * 2 + 1], 1, NULL);
        }
        else if (ATTITUDE_MODEL == 5)
        {
            // Освобождение вилок
            ReleaseSemaphore(forks[leftFork], 1, NULL);
            ReleaseSemaphore(forks[rightFork], 1, NULL);
        }

        iter--;
    }

    return 0;
}


int main() {
    setlocale(LC_ALL, "russian");
    // Инициализация семафоров для вилок
    for (int n = 0; n < 100; n++)
    {
        iter = 100;
        NUMBER_OF_MEALS = new int[NUM_PHILOSOPHERS] {0};

        for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
        {
            forks[i] = CreateSemaphore(NULL, 1, 1, NULL);  // Создаем семафор с начальным и максимальным значением 1
        }

        cout << "Создано " << NUM_PHILOSOPHERS << " вилок\n";

        if (ATTITUDE_MODEL == 4)
        {
            //Добавляем вилки
            forks = new HANDLE[NUM_PHILOSOPHERS * 2];

            for (int i = 0; i < NUM_PHILOSOPHERS*2; ++i)
            {
                forks[i] = CreateSemaphore(NULL, 1, 1, NULL);  // Создаем семафор с начальным и максимальным значением 1
            }
        }
        if (ATTITUDE_MODEL == 0)
        {
        }
        else if (ATTITUDE_MODEL == 1)
        {
            for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
                semaphores[i] = CreateSemaphore(NULL, 1, 1, NULL);  // Создаем семафор для каждого философа
            }
        }
        else if (ATTITUDE_MODEL == 2)
        {// Создание мьютекса
            mutex = CreateMutex(NULL, FALSE, NULL);
        }
        else if (ATTITUDE_MODEL == 3)
        {// Создание мьютекса
            mutex = CreateMutex(NULL, FALSE, NULL);
        }

        // Создание общего семафора для контроля захвата вилок
        forkControl = CreateSemaphore(NULL, 1, 1, NULL);


        // Создание потоков философов
        if (ATTITUDE_MODEL == 1)
        {
            for (int i = 0; i < NUM_PHILOSOPHERS; i+=2)
            {
                philosophers[i] = CreateThread(NULL, 0, Philosopher, (LPVOID)(intptr_t)i, 0, NULL);
            }
            Sleep(100);
            for (int i = 1; i < NUM_PHILOSOPHERS; i += 2)
            {
                philosophers[i] = CreateThread(NULL, 0, Philosopher, (LPVOID)(intptr_t)i, 0, NULL);
            }
        }
        else
        {
            cout << "Создано " << NUM_PHILOSOPHERS << " философов\n";
            for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
            {
                philosophers[i] = CreateThread(NULL, 0, Philosopher, (LPVOID)(intptr_t)i, 0, NULL);
            }
        }
        

        
        // Ожидание завершения потоков (они работают бесконечно в этом примере)
        WaitForMultipleObjects(NUM_PHILOSOPHERS, philosophers, TRUE, INFINITE);

        // Освобождение ресурсов (этот код в примере недостижим, но должен быть для завершения)
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
        {
            CloseHandle(forks[i]);  
            CloseHandle(philosophers[i]);
        }
        cout << n << ":-------------------------------\n";
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
        {
            cout << "id: " << i << " meal num: " << NUMBER_OF_MEALS[i] << '\n';
            
        }

        cout << "---------------------------------\n";
    }
    return 0;
}
