# SSH Native #

## Свойства ##

### АдресСервера / Host ###
**Использование:**  
Чтение и запись  
**Описание:**  
Тип: *Строка*  
Имя или IP-адрес сервера.

### Пользователь / User ###
**Использование:**  
Чтение и запись  
**Описание:**  
Тип: *Строка*  
Имя пользователя для подключения к серверу.  

### Порт / Port ###
**Использование:**  
Чтение и запись  
**Описание:**  
Тип: *Число*  
Порт для подключения к серверу.  
Значение по умолчанию: 22

### Пароль / Password ###
**Использование:**  
Только запись  
**Описание:**  
Тип: *Строка*  
Пароль пользователя.

## Методы ##

### ОписаниеОшибки / ErrorDesc ###
**Синтаксис:**  
ОписаниеОшибки()  
**Возвращаемое значение:**  
Тип: *Строка*, *Неопределено*  
**Описание:**  
Содержит текст последней ошибки выполнения метода.

### Подключиться / Connect ###
**Синтаксис:**  
Подключиться()  
**Описание:**  
Выполняет подключение и аутентификацию к серверу SSH.  
В первую очередь производится попытка аутентификации на основании ключа.
Поиск ключей осуществляется в каталоге *<каталог пользователя>/.ssh*.
В случае, если аутентификация по ключу не удалась и пароль пользователя заполнен, будет выполнена попытка парольной аутентификации.

### Выполнить / Exec ###
**Синтаксис:**  
Выполнить(<СтрокаКоманды>, <СтандартныйВывод>, <СтандартныйВыводОшибок>)  
**Параметры:**  
*  <СтрокаКоманды> (обязательный)  
   Тип: *Строка*  
   Команда для выполнения на удаленном сервере.
*  [out] <СтандартныйВывод> (необязательный)  
   Тип: *Строка*  
   Содержит результат стаднартного потока вывода (stdout) результата команды.
*  [out] <СтандартныйВыводОшибок> (необязательный)  
   Тип: *Строка*  
   Содержит результат стаднартного потока вывода ошибок (stderr) результата команды.

**Возвращаемое значение:**  
Тип: *Булево*  
Успех выполнения метода. В случае возврата значения Ложь, описание ошибки может быть получено через одноименное свойство компоненты.  
**Описание:**  
Закрывает соединение с сервером SSH.

### Прочитать / Read ###
**Синтаксис:**  
Прочитать(<Путь>, <Результат>)  
**Параметры:** 
*  <Путь> (обязательный)  
   Тип: *Строка*  
   Путь к файлу на удаленном сервере.
*  [out] <Результат> (обязательный)  
   Тип: *Двоичные данные*  
   Двоичные данные файла, полученного с удаленного сервера.

**Возвращаемое значение:**  
Тип: *Булево*  
Успех выполнения метода. В случае возврата значения Ложь, описание ошибки может быть получено через одноименное свойство компоненты.  
**Описание:**  
Выполняет получение файла с удаленного сервера по протоколу SCP.

### Записать / Write ###
**Синтаксис:**  
Записать(<Путь>, <Данные>, <Режим>)  
**Параметры:** 
*  <Путь> (обязательный)  
   Тип: *Строка*  
   Путь к файлу на удаленном сервере.
*  <Данные> (обязательный)  
   Тип: *Двоичные данные*  
   Двоичные данные файла, передаваемые на удаленный сервер.
*  <Режим> (необязательный)  
   Тип: *Число*, *Строка*  
   Определяет режим доступа к файлу, на удаленном сервере. Задается в виде строки с восьмеричным представлением или аналогичного числа в десятичной форме. Например, для режима r--r--r-- следует указывать "0444" или 292, а для rwxrwxrwx - "0777" или 511.  
   Значение по умолчанию: 0644 (rw-r--r--)

**Возвращаемое значение:**  
Тип: *Булево*  
Успех выполнения метода. В случае возврата значения Ложь, описание ошибки может быть получено через одноименное свойство компоненты.  
**Описание:**  
Выполняет отправку файла на удаленный сервер по протоколу SCP.

### ВключитьПеренаправлениеПорта / EnablePortForwarding ###
**Синтаксис:**  
ВключитьПеренаправлениеПорта(<АдресСервера>, <Порт>, <ЛокальныйПорт>)  
**Параметры:** 
*  <АдресСервера> (обязательный)  
   Тип: *Строка*  
   Адрес удаленного сервера
*  <Порт> (обязательный)  
   Тип: *Число*  
   Порт удаленного сервера   
*  [in/out] <ЛокальныйПорт> (обязательный)  
   Тип: *Число*  
   Локальный порт для приема подключений. В случае, если передано значение 0, или тип значения переменной не число,
   будет выбран свободный локальный порт и его значение будет записано в данный параметр. 

**Возвращаемое значение:**  
Тип: *Булево*  
Успех выполнения метода. В случае возврата значения Ложь, описание ошибки может быть получено через одноименное свойство компоненты.  
**Описание:**  
Включает режим перенаправления TCP порта. В этом режиме все соединения, поступающие на локальный порт, будут через
SSH соединение перенаправлены на указанный порт удаленного сервера.  
Метод может быть вызван многократно для перенаправления нескольких портов.

### ОтключитьПеренаправлениеПортов / DisablePortForwarding ###
**Синтаксис:**  
ОтключитьПеренаправлениеПортов()  
**Описание:**  
Отключает перенаправоение портов, зарегистрированных методом *ВключитьПеренаправлениеПорта*.

### ВключитьЖурнал / EnableLog ###
**Синтаксис:**  
ВключитьЖурнал(<Уровень>)  
**Параметры:** 
*  <Уровень> (необязательный)  
   Тип: *Число*  
   Уровень детализации отладочной информации.  
   Значение по умолчанию: 3 (DEBUG)

**Описание:**  
Включает запись отладочных сообщений компоненты. В связи с тем, что данные хранятся в памяти приложения, нужно понимать, что длительное использование экземпляра компоненты с включенным журналом (без его принудительной очистки) может приводить к аварийному завершению процесса 1С:Предприятия.

### ПолучитьЖурнал / GetLog ###
**Синтаксис:**  
ПолучитьЖурнал()  
**Возвращаемое значение:**  
Тип: *Строка*  
Содержимое отладочного журнала.  
**Описание:**  
Выполняет получение накопленных данных отладочного журнала. Получение журнала НЕ вызывает его очистку.

### ОчиститьЖурнал / ClearLog ###
**Синтаксис:**  
ОчиститьЖурнал()  
**Описание:**  
Выполняет очистку отладочного журнала.
