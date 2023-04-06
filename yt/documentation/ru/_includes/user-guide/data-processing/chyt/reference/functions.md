# Функции

В данном разделе собраны специфичные для CHYT функции.

## Функции для работы с YSON { #yson_functions }

### YPathType (простые типы и массивы) { #ypath_simple_types }
`YPath<Type>[Strict](yson, ypath)` &mdash; извлечь из строки `yson` значение типа `Type` по пути `ypath`.

`Type` может принимать одно из следующих значений: `Int64`, `UInt64`, `Boolean`, `Double`, `String`, `ArrayInt64`, `ArrayUInt64`, `ArrayBoolean`, `ArrayDouble`.
 
Опциональный суффикс `Strict` обозначает строгость функции.

Примеры называний функций: `YPathInt64`, `YPathStringStrict`, `YPathArrayDouble`.

Принцип работы функций состоит в следующем: 

1. Переданная строка `yson` интерпретируется как YSON-документ;
2. Происходит адресация в этом документе по пути `ypath` относительно корня документа;
3. Происходит интерпретация содержимого документа по пути `ypath` к типу `Type`.

На каждом из этапов работы данной функции могут возникнуть ошибки:

1. Строка может не представлять собой YSON-документ (такое возможно при указании строковой константы в качестве аргумента функции, но не при чтении реальных Any-колонок из {product-name});
2. Данный путь может отсутствовать в документе;
3. Данный путь может соответствовать иному типу данных, отличному от `Type` (например, вместо числового значения находится строковый литерал).

Отличие строгой версии функции (с суфиксом `Strict`) от нестрогой версии заключается в режиме обработки данных ошибок: cтрогая версия аварийно завершит работу запроса, тогда как нестрогая версия вернет в качестве результата значение по умолчанию для соответствующего типа колонки (`Null`, `0`, пустую строку или пустой список) и продолжит работу.

Примеры использования (в качестве первого аргумента везде фигурирует строковый литерал, содержащий YSON, но на его месте может быть любая any-колонка таблицы):

```sql
SELECT YPathInt64('{key1=42; key2=57; key3=29}', '/key2');
57

SELECT YPathInt64('[3;4;5]', '/1');
4

SELECT YPathString('["aa";"bb";"cc"]', '/1');
"bb"

SELECT YPathString('{a=[{c=xyz}; {c=qwe}]}', '/a/1/c');
"qwe"

SELECT YPathInt64('{key=xyz}', '/key');
0

SELECT YPathInt64Strict('{key=xyz}', '/key');
std::exception. Code: 1001, type: NYT::TErrorException, e.what() = Node /key2 has invalid type: expected one of {"int64", "uint64"}, actual "string"
    code            500
    origin          ...

SELECT YPathString('{key=3u}', '/nonexistentkey');
""

SELECT YPathArrayInt64('[1;2;3]', '');
[1, 2, 3]

SELECT YPathUInt64('42u', '');
42 
```

Предположим, что в таблице есть две колонки `lhs` и `rhs`, представляющие собой `any`-колонки, содержащие списки чисел одинаковой длины. 

<small>Таблица 1 — Пример таблицы с `any` колонками</small>

| lhs       | rhs       |
| --------- | --------- |
| [1, 2, 3] | [5, 6, 7] |
| [-1, 1]   | [1, 3]    |
| []        | []        |

Тогда можно построить скалярное произведение векторов `lhs` и `rhs` с помощью следующей конструкции:

```sql
SELECT arraySum((x, y) -> x*y, YPathArrayInt64(lhs, ''), YPathArrayInt64(rhs, '')) FROM "//path/to/table"
38
2
0
```

### YPathExtract (сложные типы) { #ypath_complex_types }
`YPathExtract[Strict](yson, ypath, type)` &mdash; извлечь из строки `yson` значение произвольного типа `type` по пути `ypath`.

Функция аналогична функциям `YPath<Type>`, но позволяет задать произвольный тип возвращаемого значения с помощью дополнительного аргумента `type`. Поддерживаются в том числе сложные составные типы, для которых нет аналога `YPath<Type>`. 

{% note warning "Внимание" %}

С помощью функции `YPathExtract` можно извлекать и простые типы. Например, результат выполнения `YPathExtract(yson, ypath, 'Int64')` будет эквивалентен результату `YPathInt64(yson, ypath)`. Однако функции `YPath<Type>` более специализированы и имеют более оптимальную реализацию, поэтому рекомендуется использовать функцию `YPathExtract` только в случае, когда для извлекаемого типа нет специализированной функции. 

{% endnote %}

Пример:
```sql
SELECT YPathExtract('[[1;2];[2;3]]', '', 'Array(Array(Int64))');
[[1, 2], [2, 3]]

SELECT YPathExtract('{a=1;b="abc"}', '', 'Tuple(a Int64, b String)');
("a": 1, "b": "abc")

SELECT YPathExtract('{x=["abc";"bbc"]}', '/x', 'Array(String)')
["abc", "bbc"]
```

### YPathRaw (необработанный yson) { #ypath_raw }

`YPathRaw[Strict](yson, ypath, [format])` &mdash; извлечь из строки `yson` по пути `ypath` необработанное значение в формате yson.

Функция может быть применена, когда тип результируемого значения неизвестен, либо различается в разных строках одной колонки.

Опциональный параметр `format` позволяет задать формат представления возвращаемого yson значения и может быть одним из следующих: `binary` (по умолчанию), `text`, `pretty`, `unescaped_text`, `unescaped_pretty`.

Примеры:

```sql
SELECT YPathRaw('{x={key=[1]}}', '/x', 'text');
'{"key"=[1;];}'

SELECT YPathRaw('{a=1}', '/b', 'text');
NULL
```

### Семейство функций YSON* { #yson_extract }

Для работы с форматом json в ClickHouse присутсвуют функции семейства `JSON*`. Для более схожей работы с обычным ClickHouse, в CHYT поддержаны полные аналоги этих функций, работающие с форматом yson.

Значение аргументов функций и их возвращаемое значение можно посмотреть в [документации ClickHouse](https://clickhouse.com/docs/ru/sql-reference/functions/json-functions).

Поддержанные аналоги на данный момент:

`YSONHas`, `YSONLength`, `YSONKey`, `YSONType`, `YSONExtractInt`, `YSONExtractUInt`, `YSONExtractFloat`, `YSONExtractBool`, `YSONExtractString`, `YSONExtract`, `YSONExtractKeysAndValues`, `YSONExtractRaw`, `YSONExtractArrayRaw`, `YSONExtractKeysAndValuesRaw`.

{% note info "Примечание" %}

В отличие от функций `YPath`, в которых нумерация элементов в массиве начинается с 0, нумерация элементов в функциях `YSON*` c единицы.

{% endnote %}

{% note warning "Внимание" %}

Данные функции реализованы общим с ClickHouse кодом и не являются оптимальными. Для ускорения вычислений на большом количестве данных рекомендуется воспользоваться более оптимальными функциями `YPath<Type>`.

{% endnote %}


Пример:

```sql
-- Эквивалентно YPathString('["aa";"bb";"cc"]', '/0')
-- Обратите внимание, нумерация начинается с 1
SELECT YSONExtractString('["aa";"bb";"cc"]', 1);
"aa"

-- Эквивалентно YPathString('{a=[{c=xyz}; {c=qwe}]}', '/a/1/c')
SELECT YSONExtractString('{a=[{c=xyz}; {c=qwe}]}', 'a', 2, 'c');
"qwe"
```

### Форматы представлени YSON

Данные из колонок типа `any` могут храниться в бинарном представлении `yson`. Поскольку читать такие данные не всегда удобно, в CHYT имеется функция `ConvertYson`, которая преобразует разные представления `yson` между собой.

`ConvertYson(yson, format)` &mdash; преобразовать строку с yson документом в указанный формат.

Всего существует пять возможных форматов представления:

- `binary`
- `text`
- `pretty`
- `unescaped_text`*
- `unescaped_pretty`*

{% note info "* Примечание" %}

В системе {product-name} предусмотрено 3 формата представления &mdash; `binary`, `text` и `pretty`.

В форматах представления `text` и `pretty` в значениях строк экранируются все символы, выходящие за пределы ascii, в том числе кириллица и различные символы в кодировке utf-8. Чтобы результирующий yson, содержащий строки с символами кириллицы, был более человекочитаемым, в CHYT были добавлены два дополнительных формата - `unescaped_text` и `unescaped_pretty`. Эти форматы отличаются от `text` и `pretty` только тем, чем в строковых значениях экранируются только служебные для формата yson символы. Остальные символы сохраняются в строках без изменений.

{% endnote %}

Примеры:

```sql
SELECT ConvertYson('{x=1}', 'text');
'{"x"=1;}'

SELECT ConvertYson('{x=1}', 'pretty');
'{
    "x" = 1;
}'

SELECT ConvertYson('{x=1}', 'binary');
'{\u0001\u0002x=\u0002\u0002;}'

SELECT ConvertYson('{x="бац"}', 'text');
'{"x"="\xD0\xB1\xD0\xB0\xD1\x86";}'

SELECT ConvertYson('{x="бац"}', 'unescaped_text');
'{"x"="бац";}'
```

### Пример работы с YSON
```sql
SELECT operation_id, task_id, YPathInt64(task_spec, '/gpu_limit') as gpu_limit, YPathInt64(task_spec, '/job_count') as job_count FROM (
    SELECT tupleElement(tasks, 1) as task_id, tupleElement(tasks, 2) as task_spec, operation_id FROM ( 
        SELECT operation_id, tasks FROM (
            Select YSONExtractKeysAndValuesRaw(COALESCE(tasks, '')) as tasks, operation_id FROM (
                SELECT YSONExtractRaw(spec, 'tasks') as tasks, operation_id
                FROM `//home/dev/chyt_examples/completed_ops`
                WHERE YSONHas(spec, 'tasks')
            )
        )
        ARRAY JOIN tasks
    )
)
ORDER BY gpu_limit DESC;
```

## Получение версии клики { #version_functions }
#### Получение версии CHYT { #chyt_version }

  `chytVersion()` &mdash; получить строковое представление версии CHYT сервера.
  
  Пример:
  ```sql
  SELECT chytVersion()
  ```

#### Получение версии {product-name} { #yt_version }

  `ytVersion()` &mdash; получить строковое представление версии кода YT, используемого в текущей версии CHYT. 

  Пример:
  ```sql
  SELECT ytVersion()
  ```

## Работа с кипарисом { #cypress }

{% note info "Примечание" %}

Все функции для работы с кипарисом являются табличными в терминах ClickHouse. Это означает, что их возвращаемое значение может использоваться вместо таблицы, например, в секции `FROM` запроса `SELECT`.

{% endnote %}


### ytListNodes { #list_nodes }
  `ytListNodes[L](dir_path, [from, [to]])` &mdash; получить список всех узлов и их атрибутов в директории `dir_path`.

  Результат содержит по одной строке на каждый узел из директории `dir_path`. Каждая строка содержит:

  - Две встроеных колонки `$key` и `$path`, хранящие имя и полный путь до узла (всегда начинается с `dir_path`). Значение в данных колонках могут отличаться от атрибутов `key` и `path`, если узел является ссылкой (тип `link`) на другой узел.

  - 3 колонки, соответствующие полям атрибута `resource_usage`: `disk_space`, `tablet_count` и `master_memory`.

  - Большое количество колонок, название которых соответствует названию атрибута узла: `key`, `path`, `account`, `owner`, `erasure_codec`, `id`, `acl` и другие. Колонки включают практически все системные атрибуты.


  {% note info "Примечание" %}

  Так как обычно пользователям не нужны все атрибуты узала, большинство колонок являются виртуальными в терминах ClickHouse. Это означает, что по умолчанию они не возвращаются через выражения `SELECT * FROM ytListNodes(...)` и `DESCRIBE ytListNodes(...)`, но могут быть запрошены явно:
  
  ```sql
  SELECT id, key_columns, * FROM ytListNodes(...)
  ```

  {% endnote %}

  По умолчанию функция `ytListNodes` не раскрывает ссылки (узлы типа `link`). Это означает, что для таких узлов в результате будут возвращены атрибуты самого узла ссылки, а не узла, на который она ссылается. Что бы изменить это поведение и раскрыть все ссылки, можно воспользоваться модификатором функции `L` (`ytListNodesL`). При этом если ссылка ведет в несуществующий узел, то она останется нераскрытой, поэтому в результате все равно могут присутсвовать строки с типом `link`, соответствующие нераскрытым ссылкам.
  
  {% note warning "Внимание" %}

  Раскрытие каждой ссылки требует дополнительного запроса к мастер серверу, поэтому исполнение функции `ytListNodeL` может быть дольше и ресурсозатратнее чем `ytListNode`. Использование `ytListNodeL` для директорий с большим количеством ссылок крайне не рекомендуется.

  {% endnote %}

  С помощью двух опциональных строковых аргументов `from` и `to` можно задать фильтрацию возвращаемых узлов по названию (колонке `$key`). Сравнение лексикографическое.

  Пример:
  ```sql
  SELECT * FROM ytListNodes('//some/dir/path');
  SELECT sum(disk_space), sum(chunk_count) FROM ytListNodes('//some/dir/path');
  SELECT * FROM ytListNodesL('//some/dir/path');
  ```

### ytListTables { #list_tables }
  `ytListTables[L](dir_path, [from, [to]])` &mdash; получить список всех таблиц и их атрибутов в директории.

  Аргументы функции и структура возвращаемого значения аналогичны функции `ytListNodes[L]`.

  Рузультат функции эквивалентен результату вызова `ytListNodes[L]` с последующей фильтрацией `WHERE type = 'table'`.

  Суффикс `L` имеет аналогичное значение и расскрывает все ссылки перед фильтрацией. Таким образом, результат выполнения `ytListTables` будет содержать только таблицы, непосредственно находящиеся в указанной директории, а результат выполнения `ytListTablesL` будет дополнительно содержать таблицы, на которые указывают ссылки из указанной директории.

  ```sql
  SELECT path, chunk_count FROM ytListTables('//dome/dir/path');
  ```

### ytListLogTables { #list_log_tables }
  `ytListLogTables(project_name_or_path, [from_datetime, [to_datetime]])` &mdash; получить непересекающийся по времени список таблиц с логами и их атрибуты из указанного проекта.

  `project_name_or_path` может содержать либо название проекта с логами (если проект лежит в директории `//logs`), либо полный путь до директории с логами.

  Функция рассмотрит таблицы с логами в четырех поддиректориях (`1h`, `30m`, `1d` и `stream/5m`), после чего объединит множества этих таблиц так, что бы интервалы времени таблиц не пересекались.

  С помощью двух опциональных аргументов `from_datetime` и `to_datetime` можно задать интересующий временной интервал логов. Таблицы с логами, которые не пересекаются по времени с указанным интервалом, будут отфильтрованы.

  {% note info "Примечание" %}

  Фильтрация по временному интервалу происходит только на уровне таблиц. Таблицы с логами из результата работы функции могут покрывать больший временной интервал, чем указанный в аргументах.

  {% endnote %}

  Пример:
  ```sql
  SELECT * FROM ytListLogTables('chyt-structured-http-proxy-log', today() - 1);
  SELECT * FROM ytListLogTables('//logs/chyt-structured-http-proxy-log', today() - 5, today() - 4);
  ```

### ytNodeAttributes { #node_attributes }
  `ytNodeAttributes(path1, [path2, [...]])` &mdash; получить атрибуты всех узлов, указанных в аргументах.

  Возвращаемое значение будет содержать по 1 строке с атрибутами на каждый указанный в аргументах путь. Если указан некоректный или несуществующий путь, будет сгенерирована ошибка.

  Структура возвращаемого значения аналогична функции `ytListNodes`.

  Пример:
  ```sql
  SELECT * FROM ytNodeAttributes('//sys', '//sys/clickhouse')
  ```

## Чтение множества таблиц { #multiple_tables_read }

{% note info "Примечание" %}

Все функции для чтения множества таблиц являются табличными в терминах ClickHouse. Это означает, что их возвращаемое значение может использоваться вместо таблицы, например, в секции `FROM` запроса `SELECT`.

{% endnote %}

### ytTables { #yt_tables }
  `ytTables(arg1, [arg2, ...])` &mdash; прочитать объединение множества таблиц, указанного в аргументах.

  Каждый аргумент может быть одним из следующих:
  - Строка, представляющая собой путь до конкретной таблицы
  - Одна из функций `ytListNodes[L]`, `ytListTables[L]`, `ytListLogTables`
  - Подзапрос, возвращающий произвольное множество путей таблиц*

  {% note warning "* Внимание" %}
  
  Передача в качестве аргумента функции подзапроса, возвращающего несколько путей, не соответствует синтаксису ClickHouse. Данная конструкция работает в текущей версии CHYT/ClickHouse, но может перестать работать в будущих версиях. Более правильным способом передачи такого подзапроса является использование функции `view`. 

  Также на данный момент известно о баге оптимизатора ClickHouse, который может некоретно переносить внешнее условие `WHERE` внутрь такого подзапроса, в результате чего может возникать ошибка `Missing columns: '<column_name>' while processing query`. Обойти данную ошибку можно с помощью настройки `enable_optimize_predicate_expression=0`.

  {% endnote %}

  Функция возвращает объединение множества всех таблиц, заданных в аргументах. Работа функции аналогичная функции [merge](https://clickhouse.com/docs/en/sql-reference/table-functions/merge) из ClickHouse, но она предоставляет более гибкую систему указания множества таблиц и оптимизирована для работы с таблицами {product-name}. 

  Если таблицы из заданного множества таблиц имеют различные схемы, то будет выведена общая схема, с помощью которой можно прочитать все заданные таблицы.

  {% cut "Алгоритм вывода общей схемы" %}

  **Совместимые типы**
  * Для каждой колонки выбирается "наиболее общий тип", к которому можно привести все типы.
  * Пример: для типов `optional<int32>` и `int64` "наиболее общим типом" будет `optional<i64>`.
  * **Обратите внимание**, что ввиду особенностей хранения данных в {product-name}, беззнаковые числовые типы являются несовместимыми со знаковыми (т.е. типы `int32` и `uint32` несовместимы).

  **Несовместимые типы**
  * Если типы колонки несовместимы в разных таблицах, то chyt будет действовать согласно опции `chyt.concat_tables.type_mismatch_mode`.
  * Пример: типы `string` и `int64` несовместимы.
  * Возможные значения опции: `drop`, `throw`, `read_as_any`.
  * По умолчанию произойдет ошибка исполнения запроса (значение `throw`).
  * Колонки разных типов можно прочитать как yson-строки, указав значение опции `read_as_any`. Это может быть неэффективно, поэтому рекомендуется структурировать свои таблицы правильно.
  * Если такие колонки не нужны для исполнения запроса, их можно отбросить, указав значение опции `drop`.

  **Отсутствующая колонка**
  * Если колонка отсуствует в одной или в нескольких входных таблицах, то chyt будет действовать согласно опции `chyt.concat_tables.missing_column_mode`.
  * Возможные значения: `drop`, `throw`, `read_as_null`.
  * По умолчанию колонка станет опциональной, а значения из таблиц, где колонка пропущена, будут прочитаны как `NULL` (значение `read_as_null`).
  * Если отсутствие колонок в некоторых таблицах неожидаемо, в таком случае можно генерировать ошибку исполнения запроса, указав значение опции `throw`.
  * Как и в случае с несовместимыми типами, такие колонки можно отбросить, указав значение опции `drop`. 
  * **Обратите внимание**, опция `missing_column_mode` влияет на поведение только в случае, если колонка пропущена в таблице со строгой схемой. Если колонка пропущена в нестрогой схеме, то она все равно может существовать в самих данных с любым типом. В этом случае делается пессимистичное предположение, что колонка присутствует с несовместимым типом, и дальнейшее поведение регулируется описанной ранее опцией `type_mismatch_mode`.

  {% endcut %}

  {% note info "Примечание" %}

  Помимо основных колонок из самих таблиц в результате также всегда присутсвуют виртуальные колонки `$table_index`, `$table_name` и `$table_path`. С помощью данных колонок можно определить, из какой именно таблицы была прочитана каждая строчка. Подробнее про виртуальные колонки можно прочитать в [соответствующем разделе](../../../../../../ru/user-guide/data-processing/chyt/yt-tables.md#virtual_columns). 

  {% endnote %}

  Пример:
  ```sql
  -- Прочитать объединение 2 таблиц:
  SELECT * FROM ytTables('//tmp/t1', '//tmp/t2')
  -- Прочитать объединение всех таблиц из директории '//tmp/dir':
  SELECT * FROM ytTables(ytListTables('//tmp/dir'))
  -- Прочитать объединение всех таблиц из директории и их названия:
  SELECT *, $table_name FROM ytTables(ytListTables('//tmp/dir'))
  -- Прочитать последнюю (лексикографически) таблицу:
  SELECT * FROM ytTables((
    SELECT max(path) FROM ytListTables('/tmp/dir')
  ))
  -- Прочитать таблицы с определенным суффиксом:
  SELECT * FROM ytTables((
    SELECT path + '/suffix' FROM ytListNodes('//tmp/dir')
  ))
  ```


### concatYtTables { #concat_yt_tables }

  `concatYtTables(table1, [table2, [...]])` &mdash; прочитать объединение нескольких {product-name} таблиц.

  Устаревший вариант функции `ytTables`. В качестве аргументов могут выступать только пути таблиц.

  Пример: 
  ```sql
  SELECT * FROM concatYtTables('//tmp/sample_table', '//tmp/sample_table2');
  ```
  
### concatYtTablesRange { #concat_yt_tables_range }

  `concatYtTablesRange(cypress_path, [from, [to]])` &mdash; прочитать объединение всех таблиц из директории `cypress_path`.

  Устаревшая функция, эквивалентная `ytTables(ytListTables(cypress_path, from, to))`.

  Пример:
  ```sql
  SELECT * FROM concatYtTablesRange("//tmp/sample_tables");
  SELECT * FROM concatYtTablesRange('//tmp/sample_tables', '2019-01-01');
  SELECT * FROM concatYtTablesRange('//tmp/sample_tables', '2019-08-13T11:00:00');
  ```