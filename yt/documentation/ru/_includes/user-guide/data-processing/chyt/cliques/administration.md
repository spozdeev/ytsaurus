# Администрирование приватной клики

В данной статье описано, как администрировать собственную клику и за какими проявлениями ее жизнедеятельности можно и нужно следить. 

## Доступы { #access }

Пользователи аутентифицируются в CHYT так же, как и в {{product-name}} — с использованием токена от {{product-name}} (подробнее см. [Аутентификация](../../../../../user-guide/storage/auth.md)). CHYT проверяет права двух видов.

Во-первых, при приёме запроса клика проверяет, что у пользователя, из-под которого совершён запрос, есть право на использование клики (право `use`). Права на клику хранятся в специальном системном узле `Access Control Object Node`, который находится по пути `//sys/access_control_object_namespaces/chyt/<alias>/principal` и создается автоматически для каждой клики при ее создании через CHYT Controller. Права на клику разграничивают доступ исключительно к вычислительным ресурсам клик.

Изменение ACL у `Access Control Object Node` происходит посредством запроса и отзыва ролей на этот объект через IDM.
Для запроса роли необходибо в [веб интерфейсе IDM](https://idm.yandex-team.ru) нажать кнопку `Запросить роль`, выбрать `YT кластер hahn / Access Control Objects / chyt / <your_clique_alias>`, далее выбрать нужную роль и список субъектов (пользователей и групп), на которых данную роль необходимо выдать.

Для клик доступны три роли:

- Use - дает права `use` на клику, которое позволяет выполнять SQL-запросы на инстансах клики.
- Manage - дает права `manage`, `read` и `remove`, которые позволяют изменять или просматривать конфигурацию клики и удалить клику соответсвенно.
- Responsible - позволяет подтверждать выдачу новых ролей на клику.

Во-вторых, при доступе к любым данным в {{product-name}} происходит проверка доступа на чтение или запись ко всем упомянутым в запросе таблицам согласно стандартному механизму ACL, используемому в {{product-name}}. Эта проверка гарантирует безопасность доступа к данным.

## Изменение ACL работающей клики { #change-acl }

Изменить ACL можно при помощи команды `update_op_parameters`, для этого необходимо:

1. Ввести алиас клики в поле **Filter Operations**, нажать **Go to operation**. Алиас начинается с символа `\*`, это имя клики.

   ![find_operation_by_alias](../../../../../../images/find_op_by_alias.png){ .center }

2. Перейти на страницу операции, скопировать `Id`.

3. Выполнить команду, которая полностью заменит acl операции. Подставить `Id` из пункта 3.

   ```bash
   yt --proxy <cluster_name> update-op-parameters --operation <operation_id> '{acl = [{subjects=[robot-1; robot-2; robot-3]; action=allow; permissions=[read]};{subjects=[<subject>]; action=allow; permissions=[read;manage]}]}'
   ```