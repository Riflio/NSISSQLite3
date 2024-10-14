#include <windows.h>
#include <stdio.h>
#include "nsis/pluginapi.h"
#include "sqlite/sqlite3.h"

#define MAX_QUERY_LENGTH 1024

/**
* @brief Выполняем запрос
* @note Пример nsissqliteplugin::execQuery 'path/to/db.sqlite' 'INSERT INTO table (col1, col2) VALUES ("val 1", "val 2");'
* @param hwndParent
* @param string_size
* @param variables
* @param stacktop
* @param extra
* @return
*/
void __declspec(dllexport) execQuery(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra, ...)
{
  EXDLL_INIT();
  TCHAR dbPath[MAX_PATH+1] ={0,};
  TCHAR dbQuery[MAX_QUERY_LENGTH] ={0,};
  sqlite3 *db =NULL;
  sqlite3_stmt *stmt =NULL;
  int errCode =0;
  //--
  switch (errCode) { case 0: {
    //-- Получаем параметры в том же порядке, в котором заданы при вызове функции. Мы обязаны забрать все параметры, ни больше, ни меньше перед выходом из функции.
    if ( popstringn(dbPath, MAX_PATH+1)!=0 ) { errCode =1; break; }
    if ( popstringn(dbQuery, MAX_QUERY_LENGTH)!=0 ) { errCode =2; break; }
    //-- Открываем БД, подготавливаем запрос
    #ifdef _UNICODE
      if ( sqlite3_open16(dbPath, &db)!=SQLITE_OK ) { errCode =3; break; }
      if ( sqlite3_prepare16_v2(db, dbQuery, -1, &stmt, NULL)!=SQLITE_OK )  { errCode =4; break; }
    #else
      if ( sqlite3_open(dbPath, &db)!=SQLITE_OK ) { errCode =3; break; }
      if ( sqlite3_prepare_v2(db, dbQuery, -1, &stmt, NULL)!=SQLITE_OK )  { errCode =4; break; }
    #endif
    //-- Выполняем
    sqlite3_step(stmt);
  }}
  //-- Обрабатываем результат
  switch (errCode) {
    case 0: { pushstring(_T("All right")); break; }
    case 1: { pushstring(_T("Empty DB path")); break; }
    case 2: { pushstring(_T("Empty DB query")); break; }
    case 3: { pushstring(_T("Unable open DB")); break; }
    case 4: { 
      pushstring(_T("Unable prepare query"));
      #ifdef _UNICODE
        pushstring(sqlite3_errmsg16(db));
      #else
        pushstring(sqlite3_errmsg(db));
      #endif
      break; 
    }
  }
  //-- Прибираемся за собой
  if ( stmt!=NULL ) { sqlite3_finalize(stmt); stmt =NULL; }
  if ( db!=NULL ) { sqlite3_close(db); db =NULL; }
  //-- Выходим
  pushint(errCode);
}

/**
* @brief Преобразуем строку из utf-8 в widechar
* @param utf8
* @return Строка в widechar. ВАЖНО - освобождает её вызывающий.
*/
static wchar_t* convertUTF8ToUTF16(const char* utf8)
{
  int utf8Len =MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
  if ( utf8Len==0 ) { return NULL; }
  wchar_t *utf16 =(wchar_t*)malloc(utf8Len*sizeof(wchar_t));
  if ( utf16==NULL ) { return NULL; }
  int res =MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf8Len);
  if ( res==0 ) { free(utf16); return NULL; }
  return utf16;
}

/**
* @brief Импортируем SQL дамп целиком
* @note Пример nsissqliteplugin::importDump 'path/to/db.sqlite' 'path/to/dump.sql'
* @note Файл с дампом должен быть в кодировке UTF-8
* @param hwndParent
* @param string_size
* @param variables
* @param stacktop
* @param extra
* @return
*/
void __declspec(dllexport) importDump(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra, ...)
{
  EXDLL_INIT(); //-- Инициализация API
  TCHAR dbPath[MAX_PATH+1] ={0,};
  TCHAR dumpFilePath[MAX_PATH+1] ={0,};
  char *dumpFileContent =NULL;
  FILE *dumpFile =NULL;
  sqlite3 *db =NULL;
  int errCode =0;
  char *sqlErrMsg =NULL;
  //--
  switch (errCode) { case 0: {
    //-- Получаем параметры в том же порядке, в котором заданы при вызове функции. Мы обязаны забрать все параметры, ни больше, ни меньше перед выходом из функции.
    if ( popstringn(dbPath, MAX_PATH+1)!=0 ) { errCode =1; break; }
    if ( popstringn(dumpFilePath, MAX_PATH+1)!=0 ) { errCode =2; break; }
    //-- Будем читать весь файл целиком (построчно не можем, т.к. запрос может занимать несколько строк). Открываем.
    dumpFile =_tfopen(dumpFilePath, _T("rb")); //-- Открываем в бинарном виде, что бы со строками не мучаться
    if ( dumpFile== NULL ) { errCode =3; break; }
    //-- Определение размера файла
    fseek(dumpFile, 0, SEEK_END);
    int dumpFileSize =ftell(dumpFile);
    fseek(dumpFile, 0, SEEK_SET);
    //-- Выделяем память для буфера
    dumpFileContent =(char *)malloc(dumpFileSize/sizeof(char)+1); //-- +1 на нуль завершающий
    if ( dumpFileContent==NULL ) { errCode =4; break; }
    //-- Читаем целиком
    fread(dumpFileContent, sizeof(char), dumpFileSize/sizeof(char), dumpFile);
    dumpFileContent[dumpFileSize/sizeof(char)] ='\0'; //-- Завершаем явно нулём
    //-- Открываем БД, выполняем запрос
    #ifdef _UNICODE
      if ( sqlite3_open16(dbPath, &db)!=SQLITE_OK ) { errCode =5; break; }
    #else
      if ( sqlite3_open(dbPath, &db)!=SQLITE_OK ) { errCode =5; break; }
    #endif
    if ( sqlite3_exec(db, dumpFileContent, NULL, NULL, &sqlErrMsg)!=SQLITE_OK )  { errCode =6; break; }
  }}
  
  //-- Обрабатываем результат
  switch (errCode) {
    case 0: { pushstring(_T("All right")); break; }
    case 1: { pushstring(_T("Empty DB path")); break; }
    case 2: { pushstring(_T("Empty dump file path")); break; }
    case 3: { pushstring(_T("Unable open dump file path")); break; }
    case 4: { pushstring(_T("Internal error")); break; }
    case 5: { pushstring(_T("Unable open DB")); break; }
    case 6: { 
      pushstring(_T("Query error"));
      #ifdef _UNICODE
      	if ( sqlErrMsg!=NULL ) {
      	  wchar_t *sqlErrMsg16 =convertUTF8ToUTF16(sqlErrMsg);
      	  if ( sqlErrMsg16!=NULL ) { pushstring(sqlErrMsg16); free(sqlErrMsg16); }
      	  else { pushstring(_T("Unable convert error message")); }
        } else { 
          pushstring(_T("Unknown error")); 
        }
      #else
        pushstring(sqlErrMsg);
      #endif
      break;
    }
  }
  //-- Прибираемся за собой
  if ( dumpFileContent!=NULL ) { free(dumpFileContent); dumpFileContent =NULL; }
  if ( dumpFile!=NULL ) { fclose(dumpFile); dumpFile =NULL; }
  if ( sqlErrMsg!=NULL ) { sqlite3_free(sqlErrMsg); sqlErrMsg =NULL; }
  if ( db!=NULL ) { sqlite3_close(db); db =NULL; }
  //-- Выходим
  pushint(errCode);
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}