#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <vector>

// --- Глобальные переменные ---
// Общий "мешок" с монетами
const int INITIAL_COINS = 101; 
int coins = INITIAL_COINS;

// "Карманы" каждого вора
int bob_coins = 0;
int tom_coins = 0;

// Мьютекс для защиты всех общих переменных
std::mutex coin_mutex;

// Функция, описывающая поведение вора
void coin_sharing(const std::string& name, int& thief_coins, int& companion_coins)
{
    const int stop_limit = (INITIAL_COINS % 2 == 0) ? 0 : 1;

    // Воры делят монеты, пока в мешке есть что делить (больше одной монеты)
    while (true)
    {
        // Блокируем мьютекс перед доступом к общим данным
        coin_mutex.lock();

        // Проверяем условие выхода из цикла
        if (coins <= stop_limit) {
            // Если монет осталось 1 или меньше, дележка окончена
            coin_mutex.unlock(); // Не забываем разблокировать перед выходом!
            break;
        }

        // Ключевая логика: вор берет монету, только если у него монет
        // не больше, чем у напарника. Это обеспечивает очередность.
        if (thief_coins <= companion_coins)
        {
            // Забираем монету из мешка
            coins--;
            // Кладем к себе в карман
            thief_coins++;

            // Выводим текущее состояние
            std::cout << name << " took a coin. State -> Bob: " << bob_coins
                << ", Tom: " << tom_coins << ", Pile: " << coins << std::endl;
        }

        // Разблокируем мьютекс, чтобы дать другому потоку шанс
        coin_mutex.unlock();

        // Небольшая пауза, чтобы снизить нагрузку на процессор от "busy-waiting".
        // Без нее потоки будут бешено крутиться в цикле, постоянно блокируя и разблокируя мьютекс.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    std::cout << "Starting the heist with " << coins << " coins..." << std::endl;
    std::cout << "---------------------------------------------" << std::endl;

    // Создаем потоки для Боба и Тома.
    // Для Боба: его карман - thief_coins, карман Тома - companion_coins.
    // Для Тома: его карман - thief_coins, карман Боба - companion_coins.
    // std::ref используется для передачи переменных по ссылке в потоки.
    std::thread bob_thread(coin_sharing, "Bob", std::ref(bob_coins), std::ref(tom_coins));
    std::thread tom_thread(coin_sharing, "Tom", std::ref(tom_coins), std::ref(bob_coins));

    // Ожидаем завершения работы обоих потоков
    bob_thread.join();
    tom_thread.join();

    // Выводим финальные результаты
    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "The sharing is over. Final results:" << std::endl;
    std::cout << "Bob's total: " << bob_coins << " coins." << std::endl;
    std::cout << "Tom's total: " << tom_coins << " coins." << std::endl;
    std::cout << "Coins left for the dead man: " << coins << " coin." << std::endl;
    std::cout << "Total checksum: " << (bob_coins + tom_coins + coins) << " coins." << std::endl;

    return 0;
}