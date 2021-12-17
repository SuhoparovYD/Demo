//user interface data public
char *FullMenu[6][9]={
 {"Агрегат..       ","Встряхивание..  ","Настройка..     ",
  "Алгоритм..      ","Отладка..       "},
 {"Ограничение тока","Ограничение напр","Огранич.максим.н",
  "Ограничение угла","Ручной режим    ","Количество пауз ",
  "Кол-во импульсов","Угол упр.в паузе","Дист.управление "},
 {"Состояние       ","Время паузы Ч:М ","Время работы, c ",
  "Расчет интервала","Отк.пит.при сбое","Прореживание пит"},
 {"Изменить пароль ","Запись настроек ","Номер поля      ",
  "Напр.рег.пробоя ","Напр.регист.К.З.","Ток при К.З.    ",
  "Макс. напр. Х.Х.","Ток при Х.Х.    ","Пауза деионизац."},
 {"Оптимизация Vu  ","Скорость/ Период","Отраб.втор.проб.",
  "Метод гашения ОК","Амплит.пульсаций"},
 {"Отладка Агр.пит.","Отладка Встрях. ","Отладка Сети.   ",
  "Отладка конст.Т ","Отладка оптим.V ","Неравнофазность ",
  "Ток Авар. откл. ","Макс.форсировка "}};

/* В режиме индикации 0 показывается информация с наложением байт
Состояния(Работа /Пауза),Исправность(Сбои),а в соответствующих режимах только этот байт */

int sizeMenu[]={4,8,4,8,1,7};   // in n-1

char *Icon[6][9]={{"А","В","Н","А","О"},
  {"Ток огр. ","Напр.огр.","Макс.напр","Огр. угла","Ручн.угол",
   "К-во пауз","К-во имп.","Угол ЧПП ","Дист.упр. "},
  {"Состоян.","Пауза","Работа","Расчет","Окл.питания ","Прореж.пит. "},
  {"Пароль","Запись Cr/Esc","Номер поля","Напр.проб.",
   "Напр. К.З.","Ток К.З.  ", "Напр. Х.Х.","Ток Х.Х.  ","Пауза деио"},
  {"Опт. Vu ","Скор/П-д","Втор.пробой","Поиск ОК","Интервал ",
   "Время гаш.","Гашен.ОК","Ампл.пульс"},
  {"А","В","С","T","V","In","Ток.ав.от","Макс.форс"}};

char *opTay[5] = {"const V","optim V","const T","optim T",""};
char *dubleSp[4] = {" 1/2"," 1/n","  1 ",""};
char *yesNo[3] = {"Нет","Да ",""};
char *findOK[4]= {"  ВАХ  ","Пульсац","Разряд ",""};
char *metOK[3] = {"Прореж ","Огранич",""};
char *Have[3]  = {"Нет  ","Есть ",""};

/*char *rPar[]= { "Пылеемк.  ", "Длина поля", "М.эл.расст",
                "Скор. газа", "Запыленн. ", "КПД       ",
                "Кол-во. полей","Номер поля   ", "Время"};*/
char *rPar[]= { "Mo, кг/м2 ", "Lп, м     ", "d , мм    ",
                "Vг, м/с   ", "Zвх, г/м3 ", "КПД       ",
                "N полей   ", "n поля    ", "T, мин"};


