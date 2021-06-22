#ifndef LOCALE_H
#define LOCALE_H

#include "Keywords.h"

#ifndef _LOCALIZED_
#define _LOCALE_ENG_
#endif

#ifdef _LOCALE_ENG_
#define _ROSSA_VERSION_LONG_ "Rossa " _ROSSA_VERSION_ "\nCopyright 2020 - BSD 3-Clause License\nAuthors:\n * Benjamin Park / parkbenjamin2@gmail.com"
#define _ROSSA_INTERPRETER_START_ "Rossa " _ROSSA_VERSION_ " Interpreter"
#define _STANDARD_LIBRARY_LOADED_ "Standard Library Loaded"
#define _STANDARD_LIBRARY_LOAD_FAIL_ "Failed to load Standard Library: "
#define _FAILURE_FILEPATH_ "Cannot find path to file: "
#define _FILE_NOT_FOUND_ "File `{0}` cannot be found"
#define _OPTION_NO_STD_ "Option --no-std (-ns) used; Standard Library not loaded"
#define _UNDECLARED_VARIABLE_ERROR_ "Variable `{0}` is not declared within scope"
#define _INVALID_OBJECT_TYPE_ "Invalid " KEYWORD_OBJECT " type"
#define _UNKNOWN_BUILT_CALL_ "Unknown built in function: {0}"
#define _UNKNOWN_BINARY_OP_ "Unknown binary operator: {0}"
#define _UNKNOWN_UNARY_OP_ "Unknown unary operator: {0}"
#define _FAILURE_INSTANTIATE_OBJECT_ "Cannot instantiate a non-struct " KEYWORD_OBJECT
#define _CANNOT_ENTER_DICTIONARY_ "Cannot enter " KEYWORD_DICTIONARY " with given value"
#define _CANNOT_INDEX_OBJECT_ "Cannot index a non-static, non-instantiated " KEYWORD_OBJECT
#define _CANNOT_INDEX_VALUE_ "Value is not an index-able type"
#define _EXTERN_NOT_DEFINED_ "External function `{0}` is not defined"
#define _FUNCTION_PARAM_ERROR_ "Parametric values must either be " KEYWORD_FUNCTION " or " KEYWORD_OBJECT "; given type: `{0}`"
#define _FAILURE_LENGTH_ "Cannot get length of value"
#define _FAILURE_SIZE_ "Cannot get size of value"
#define _FAILURE_EXTEND_ "Cannot extend a statically declared " KEYWORD_OBJECT
#define _FAILURE_STR_TO_NUM_ KEYWORD_STRING " `{0}` cannot be converted to " KEYWORD_NUMBER
#define _FAILURE_CONVERT_ "Cannot convert to given type"
#define _FAILURE_ALLOC_ "Cannot initialize a " KEYWORD_ARRAY " with size < 0"
#define _FAILURE_TO_STR_ "Cannot convert values into " KEYWORD_STRING
#define _FAILURE_STRING_FORMAT_ "Elements to be parsed remain in " KEYWORD_STRING
#define _NOT_NUMBER_ "Value is not of type `" KEYWORD_NUMBER "`"
#define _NOT_POINTER_ "Value is not of type `" KEYWORD_POINTER "`"
#define _NOT_DICTIONARY_ "Value is not of type `" KEYWORD_DICTIONARY "`"
#define _NOT_VECTOR_ "Value is not of type `" KEYWORD_ARRAY "`"
#define _NOT_STRING_ "Value is not of type `" KEYWORD_STRING "`"
#define _NOT_BOOLEAN_ "Value is not of type `" KEYWORD_BOOLEAN "`"
#define _NOT_OBJECT_ "Value is not of type `" KEYWORD_OBJECT "`"
#define _NOT_TYPE_ "Value is not of type `" KEYWORD_TYPE "`"
#define _NOT_FUNCTION_ "Value is not of type `" KEYWORD_FUNCTION "`"
#define _FUNCTION_VALUE_NOT_EXIST_ KEYWORD_FUNCTION " does not exist for given value type"
#define _FUNCTION_ARG_SIZE_FAILURE_ KEYWORD_FUNCTION " overloads do not encompass given argument size"
#define _INDEX_OUT_OF_BOUNDS_ "Index out of bounds: size {0}, got {1}"
#define _FAILURE_PARSE_CODE_ "Failure to parse code"
#define _EXPECTED_ERROR_ "Expected `{0}`"
#define _EXPECTED_FUNCTION_PARAM_ "Expected " KEYWORD_FUNCTION " parameter"
#define _EXPECTED_BASE_TYPE_ "Expected base type for prototypic " KEYWORD_FUNCTION " declaration"
#define _EXPECTED_FUNCTION_NAME_ "Expected " KEYWORD_FUNCTION " name"
#define _EXPECTED_FUNCTION_SIG_ "Expected " KEYWORD_FUNCTION " signature"
#define _EXPECTED_OBJECT_NAME_ "Expected " KEYWORD_OBJECT " declaration"
#define _EXPECTED_BASE_CAST_ "Expected base type for value casting"
#define _EXPECTED_RH_ "Expected right-hand expression"
#define _UNEXPECTED_TERMINATION_ "Expression terminated unexpectedly"
#define _NOT_UNARY_OP_ "Operator is not applicable to unary operation"
#define _VALUE_KEY_PAIR_ERROR_ "Value cannot be used to generate key-value pair"
#define _EXPECTED_AFTER_ "Expected expression after `{0}`"
#define _EXPECTED_IDF_ "Expected identifier"
#define _EXPECTED_EXPR_ "Expected expression"
#define _EXPECTED_FILE_ "Expected filepath after `" KEYWORD_LOAD "`"
#define _UNDECLARED_OPERATOR_ERROR_ "Operator `{0}` is not declared for the given types"
#define _STACK_TRACE_MORE_ " ... ({0} more) ..."
#define _EXTERNAL_LIBRARY_NOT_EXIST_ "External library does not exist: `{0}`"
#define _EXPORT_FUNCTION_NOT_FOUND_ "No export function found in library: `{0}`"
#define _LIBRARY_NOT_IN_MEMORY_ "Library has not yet been loaded into memory: `{0}`"
#define _LIBRARY_FUNCTION_NOT_EXIST_ "Library `{0}` has not exported function: `{1}`"
#define _INCOMPATIBLE_VECTOR_SIZES_ "Size of " KEYWORD_ARRAY " values are not compatible"
#define _CANNOT_MAKE_CONST_ "Expression cannot be parsed in a constant manner"
#endif

#ifdef _LOCALE_ITA_
#define _ROSSA_VERSION_LONG_ "Rossa " _ROSSA_VERSION_ "\nDiritto d'autore 2020 - BSD 3-Clause Licenza\nAutori:\n * Benjamin Park / parkbenjamin2@gmail.com"
#define _ROSSA_INTERPRETER_START_ "Rossa " _ROSSA_VERSION_ " Interprete"
#define _STANDARD_LIBRARY_LOADED_ "Libreria Standard è Caricata"
#define _STANDARD_LIBRARY_LOAD_FAIL_ "Caricamento di Libreria Standard è Bocciato: "
#define _FAILURE_FILEPATH_ "Non si può trovare il percorso del file: "
#define _FILE_NOT_FOUND_ "Non si può trovare il file di `{0}`"
#define _OPTION_NO_STD_ "Opzione --no-std (-ns) è usata; Libreria Standard non è Caricata"
#define _UNDECLARED_VARIABLE_ERROR_ "Variabile `{0}` non si declarava nell'ambito"
#define _INVALID_OBJECT_TYPE_ "Tipo d'Oggetto [" KEYWORD_OBJECT "] è irrito"
#define _UNKNOWN_BUILT_CALL_ "Funzione predefinita è sconosciuta: {0}"
#define _UNKNOWN_BINARY_OP_ "Operatore binario è sconosciuto: {0}"
#define _UNKNOWN_UNARY_OP_ "Operatore unario è sconosciuto: {0}"
#define _FAILURE_INSTANTIATE_OBJECT_ "Non si replica un Oggetto [" KEYWORD_OBJECT "] che si non definisce con `struct`"
#define _CANNOT_ENTER_DICTIONARY_ "Non si può entrare nel'un Diccionario [" KEYWORD_DICTIONARY "] col valore dato"
#define _CANNOT_INDEX_OBJECT_ "Non si può indicizzare un Oggetto [" KEYWORD_OBJECT "] che si non definisce con `static`, o non è replcato"
#define _CANNOT_INDEX_VALUE_ "Valore è un tipo che non si può indicizzare"
#define _EXTERN_NOT_DEFINED_ "Funzione straniera `{0}` non è definita"
#define _FUNCTION_PARAM_ERROR_ "Valori parametrici devono essere una Funzione [" KEYWORD_FUNCTION "] o un Oggetto [" KEYWORD_OBJECT "]; il tipo dato: `{0}`"
#define _FAILURE_LENGTH_ "Valore non ha lunghezza"
#define _FAILURE_SIZE_ "Valore non ha taglia"
#define _FAILURE_EXTEND_ "Non si può stendere un Oggetto [" KEYWORD_OBJECT "] che si declara con `static`"
#define _FAILURE_STR_TO_NUM_ "Non si può convertire Stringa [" KEYWORD_STRING "] `{0}` a Numero [" KEYWORD_NUMBER "]"
#define _FAILURE_CONVERT_ "Non si può convertire nel tipo dato"
#define _FAILURE_ALLOC_ "Non si può creare un Vettore [" KEYWORD_ARRAY "] colla lunghezza meno di 0"
#define _FAILURE_TO_STR_ "Non si può convertire i valori in una Stringa [" KEYWORD_STRING "]"
#define _FAILURE_STRING_FORMAT_ "Ci sono parti rimanenti per analisi nella Stringa [" KEYWORD_STRING "]"
#define _NOT_NUMBER_ "Il tipo di valore non è `Numero` [" KEYWORD_NUMBER "]"
#define _NOT_POINTER_ "Il tipo di valore non è `Puntatore` (" KEYWORD_POINTER ")"
#define _NOT_DICTIONARY_ "Il tipo di valore non è `Diccionario` [" KEYWORD_DICTIONARY "]"
#define _NOT_VECTOR_ "Il tipo di valore non è `Vettore` [" KEYWORD_ARRAY "]"
#define _NOT_STRING_ "Il tipo di valore non è `Stringa` [" KEYWORD_STRING "]"
#define _NOT_BOOLEAN_ "Il tipo di valore non è `Booleano` [" KEYWORD_BOOLEAN "]"
#define _NOT_OBJECT_ "Il tipo di valore non è `Oggetto` [" KEYWORD_OBJECT "]"
#define _NOT_TYPE_ "Il tipo di valore non è `Tipo` [" KEYWORD_TYPE "]"
#define _NOT_FUNCTION_ "Il tipo di valore non è `Funzione` [" KEYWORD_FUNCTION "]"
#define _FUNCTION_VALUE_NOT_EXIST_ "Non c'è Funzione [" KEYWORD_FUNCTION "] scritta per il tipo di valore dato"
#define _FUNCTION_ARG_SIZE_FAILURE_ "Sovraccarichi della Funzione [" KEYWORD_FUNCTION "] non consentono la taglia delgi parametri"
#define _INDEX_OUT_OF_BOUNDS_ "Indice è fuori dello scopo: lunghezza di {0}, ricevuto {1}"
#define _FAILURE_PARSE_CODE_ "Processamento è Bocciato"
#define _EXPECTED_ERROR_ "Si aspettava di `{0}`"
#define _EXPECTED_FUNCTION_PARAM_ "Si aspettava di parametri della Funzione [" KEYWORD_FUNCTION "]"
#define _EXPECTED_BASE_TYPE_ "Si aspettava tipo basico nella declarazione d'una Funzione [" KEYWORD_FUNCTION "] prototipica"
#define _EXPECTED_FUNCTION_NAME_ "Si aspettava il nome della Funzione [" KEYWORD_FUNCTION "]"
#define _EXPECTED_FUNCTION_SIG_ "Si aspettava il signatorio della Funzione [" KEYWORD_FUNCTION "]"
#define _EXPECTED_OBJECT_NAME_ "Si aspettava la declarazione d'un Oggetto [" KEYWORD_OBJECT "]"
#define _EXPECTED_BASE_CAST_ "Conversione di tipo richiede tipi basici"
#define _EXPECTED_RH_ "Si aspettava un espressione alla destra"
#define _UNEXPECTED_TERMINATION_ "L'espression terminava inaspettatamente"
#define _NOT_UNARY_OP_ "L'operatore non è applicabile a una operazione unaria"
#define _VALUE_KEY_PAIR_ERROR_ "Nella creazione d'un paio chiave-valore non può usare il valore dato"
#define _EXPECTED_AFTER_ "Si aspettava espressione dopo `{0}`"
#define _EXPECTED_IDF_ "Si aspettava identificatore"
#define _EXPECTED_EXPR_ "Si aspettava espressione"
#define _EXPECTED_FILE_ "Si aspettava un percorso dopo `" KEYWORD_LOAD "`"
#define _UNDECLARED_OPERATOR_ERROR_ "L'operatore `{0}` non si declarava per i tipi dati"
#define _STACK_TRACE_MORE_ " ... ({0} più) ..."
#define _EXTERNAL_LIBRARY_NOT_EXIST_ "Libreria esterna non si può trovata: `{0}`"
#define _EXPORT_FUNCTION_NOT_FOUND_ "Libreria non contiene una funziona esportatrice: `{0}`"
#define _LIBRARY_NOT_IN_MEMORY_ "Libreria non s'è caricata in memoria: `{0}`"
#define _LIBRARY_FUNCTION_NOT_EXIST_ "Librera `{0}` non ha esportata la funzione: `{1}`"
#define _INCOMPATIBLE_VECTOR_SIZES_ "Taglia dei valori Vettori [" KEYWORD_ARRAY "] non ha compatabilità"
#define _CANNOT_MAKE_CONST_ "Non si può processare l'espressione con un modo costante"
#endif

#ifdef _LOCALE_LAT_
#define _ROSSA_VERSION_LONG_ "Rossa " _ROSSA_VERSION_ "\nDīrēctus Auctōris ex MMXX - Licentiā BSD III-Ideae\nAuctōrēs:\n * Beniamin Parchus / parkbenjamin2@gmail.com"
#define _ROSSA_INTERPRETER_START_ "Rossa " _ROSSA_VERSION_ " Interpretātrum"
#define _STANDARD_LIBRARY_LOADED_ "Bibliothēca Generālis Legitur"
#define _STANDARD_LIBRARY_LOAD_FAIL_ "Bibliothēca Generālis nōn legī poterat: "
#define _FAILURE_FILEPATH_ "Ad scāpum via nōn legī poterat: "
#define _FILE_NOT_FOUND_ "Scāpus quī `{0}` nōminātur nōn invenīrī poterat"
#define _OPTION_NO_STD_ "Optiō --no-std (-ns) dabātur; Generālis Bibliothēca nōn legitur"
#define _UNDECLARED_VARIABLE_ERROR_ "Variābilis quī `{0}` nōminātur nōn in dominiō dēclārātur"
#define _INVALID_OBJECT_TYPE_ "Dēclārātiō Strūctūrae [" KEYWORD_OBJECT "] irrita est"
#define _UNKNOWN_BUILT_CALL_ "Appellātiō prōcessiōnis compīlātae ignōta est: {0}"
#define _UNKNOWN_BINARY_OP_ "Iussus bīnārius ignōtus est: {0}"
#define _UNKNOWN_UNARY_OP_ "Iussus ūnārius ignōtus est: {0}"
#define _FAILURE_INSTANTIATE_OBJECT_ "Strūctūram [" KEYWORD_OBJECT "] quae cum `struct` nōn dēclārātur nōn fierī potest"
#define _CANNOT_ENTER_DICTIONARY_ "Nōn potes in Mātrīce [" KEYWORD_DICTIONARY "] cum valōre datō inīre"
#define _CANNOT_INDEX_OBJECT_ "Nōn potes in Strūctūra [" KEYWORD_OBJECT "] quae et cum `static` nōn dēclārātur et nōn ipsa dēclārāta est intrāre"
#define _CANNOT_INDEX_VALUE_ "Sīcut genus quod intrāre nōn potes dēclārātus est valor"
#define _EXTERN_NOT_DEFINED_ "Prōcessiō extrāria quae `{0}` nōminātur nōn scrībitur"
#define _FUNCTION_PARAM_ERROR_ "Valōrēs parametreī dandī sunt sicut Prōcessiōnem [" KEYWORD_FUNCTION "] aut Strūctūram [" KEYWORD_OBJECT "]; quod datum: `{0}`"
#define _FAILURE_LENGTH_ "Valor longitūdinem nōn habet"
#define _FAILURE_SIZE_ "Valor magnitūdinem nōn habet"
#define _FAILURE_EXTEND_ "A Strūctūrā [" KEYWORD_OBJECT "] quae cum `static` dēclārātur novum nōn prōgignī potes"
#define _FAILURE_STR_TO_NUM_ "Seriēs [" KEYWORD_STRING "] `{0}` in Numerum mūtārī nōn potest"
#define _FAILURE_CONVERT_ "Ad genus nōn mūtārī potest"
#define _FAILURE_ALLOC_ "Tabula [" KEYWORD_ARRAY "] cum magnitūdine < 0 nōn creārī potest"
#define _FAILURE_TO_STR_ "Valōres in Seriem [" KEYWORD_STRING "] mūtārī nōn possunt"
#define _FAILURE_STRING_FORMAT_ "In Seriē [" KEYWORD_STRING "] manent prō computātiōne incomplētās adhūc partēs"
#define _NOT_NUMBER_ "Cum genere `Numerus` [" KEYWORD_NUMBER "] nōn dēclārātur valor"
#define _NOT_POINTER_ "Cum genere `Index` [" KEYWORD_POINTER "] nōn dēclārātur valor"
#define _NOT_DICTIONARY_ "Cum genere `Mātrix` [" KEYWORD_DICTIONARY "] nōn dēclārātur valor"
#define _NOT_VECTOR_ "Cum genere `Tabula` [" KEYWORD_ARRAY "] nōn dēclārātur valor"
#define _NOT_STRING_ "Cum genere `Seriēs` [" KEYWORD_STRING "] nōn dēclārātur valor"
#define _NOT_BOOLEAN_ "Cum genere `Logicum` [" KEYWORD_BOOLEAN "] nōn dēclārātur valor"
#define _NOT_OBJECT_ "Cum genere `Strūctūra` [" KEYWORD_OBJECT "] nōn dēclārātur valor"
#define _NOT_TYPE_ "Cum genere `Genus` [" KEYWORD_TYPE "] nōn dēclārātur valor"
#define _NOT_FUNCTION_ "Cum genere `Prōcessiō` [" KEYWORD_FUNCTION "] nōn dēclārātur valor"
#define _FUNCTION_VALUE_NOT_EXIST_ "Prō valōre datō nōn est Prōcessiō [" KEYWORD_FUNCTION "]"
#define _FUNCTION_ARG_SIZE_FAILURE_ "Prō magnitūdine parametrōrum datōrum scrīpta nōn est Prōcessiō [" KEYWORD_FUNCTION "]"
#define _INDEX_OUT_OF_BOUNDS_ "Postulātiō data extrā magnitūdinem est: habet {0}, {1} dabātur"
#define _FAILURE_PARSE_CODE_ "Compilātiō prōcessūs dēfēcit"
#define _EXPECTED_ERROR_ "`{0}` exspectābātur"
#define _EXPECTED_FUNCTION_PARAM_ "Parametrum Prōcessiōnis [" KEYWORD_FUNCTION "] exspectābātur"
#define _EXPECTED_BASE_TYPE_ "Genus simplex in dēclārātiō Prōcessiōnis [" KEYWORD_FUNCTION "] successae exspectābātur"
#define _EXPECTED_FUNCTION_NAME_ "Nōmen  Prōcessiōnis [" KEYWORD_FUNCTION "] exspectābātur"
#define _EXPECTED_FUNCTION_SIG_ "Signum Prōcessiōnis [" KEYWORD_FUNCTION "] exspectābātur"
#define _EXPECTED_OBJECT_NAME_ "Dēclārātiō Strūctūrae [" KEYWORD_OBJECT "] exspectābātur"
#define _EXPECTED_BASE_CAST_ "Genus simplex in iussō mūtātiōnis exspectābātur"
#define _EXPECTED_RH_ "In dextrō ēnūntiātiō exspectābātur"
#define _UNEXPECTED_TERMINATION_ "Ad fīnem suam attigit ēnūntiātiō inexspectāte"
#define _NOT_UNARY_OP_ "In iussō ūnāriō nōn agī potest"
#define _VALUE_KEY_PAIR_ERROR_ "In coniugiō Mātrīcis [" KEYWORD_DICTIONARY "] nōn accēdere potest valor"
#define _EXPECTED_AFTER_ "Post `{0}` ēnūntiātiō exspectābātur"
#define _EXPECTED_IDF_ "Nōmen exspectābātur"
#define _EXPECTED_EXPR_ "Ēnūntiātiō exspectābātur"
#define _EXPECTED_FILE_ "Post `" KEYWORD_LOAD "` ad scāpum via exspectābātur"
#define _UNDECLARED_OPERATOR_ERROR_ "Iussus quī `{0}` nōminātur nōn dēclārātur cum valōribus datīs"
#define _STACK_TRACE_MORE_ " ... (plūs {0}) ..."
#define _EXTERNAL_LIBRARY_NOT_EXIST_ "Bibliothēca externa nōn exsistit: `{0}`"
#define _EXPORT_FUNCTION_NOT_FOUND_ "In bibliothēcā nōn invenītur prōcessiō exportātiōnis: `{0}`"
#define _LIBRARY_NOT_IN_MEMORY_ "In memoriā nōn lēcta est bibliothēcā: `{0}`"
#define _LIBRARY_FUNCTION_NOT_EXIST_ "Nōn est exportāta ē bibliothēcā `{0}` prōcessiō: `{1}`"
#define _INCOMPATIBLE_VECTOR_SIZES_ "Magnitūdinēs valōrum Tabulae [" KEYWORD_ARRAY "] nōn congruunt"
#define _CANNOT_MAKE_CONST_ "Ēnūntiātiō nōn cōnstante prōcēditur"
#endif

#ifdef _LOCALE_JPN_
#define _ROSSA_VERSION_LONG_ "ロッサ・" _ROSSA_VERSION_ "\n2020版権・BSD 3-Clause License\n作者:\n * パーク・ベンジャミン / parkbenjamin2@gmail.com"
#define _ROSSA_INTERPRETER_START_ "ロッサ・" _ROSSA_VERSION_ "のインタプリタ"
#define _STANDARD_LIBRARY_LOADED_ "一般的なライブラリーはロードされた"
#define _STANDARD_LIBRARY_LOAD_FAIL_ "一般的なライブラリーのロードは駄目："
#define _FAILURE_FILEPATH_ "ファイルの道は不明："
#define _FILE_NOT_FOUND_ "ファイルの「{0}」は見つけられなかった"
#define _OPTION_NO_STD_ "「--no-stdか-ns」は使われた・一般的なライブラリーはされなかった"
#define _UNDECLARED_VARIABLE_ERROR_ "変数の「{0}」は有効範囲の中で見られない"
#define _INVALID_OBJECT_TYPE_ "クラス（" KEYWORD_OBJECT "）の種類は無効"
#define _UNKNOWN_BUILT_CALL_ "先天の関数の「{0}」は不明"
#define _UNKNOWN_BINARY_OP_ "二項の演算子の「{0}」は不明"
#define _UNKNOWN_UNARY_OP_ "一項の演算子の「{0}」は不明"
#define _FAILURE_INSTANTIATE_OBJECT_ "「struct」で出来なかったクラス（" KEYWORD_OBJECT "）の実体化は駄目"
#define _CANNOT_ENTER_DICTIONARY_ "くれた値を使ってテーブル（" KEYWORD_DICTIONARY "）の入りは駄目"
#define _CANNOT_INDEX_OBJECT_ "「struct」で出来なかったか実体化されなかったらクラス（" KEYWORD_OBJECT "）の入りはダメ"
#define _CANNOT_INDEX_VALUE_ "入られラれない種類として値は定義した"
#define _EXTERN_NOT_DEFINED_ "外の関数の「{0}」は不明"
#define _FUNCTION_PARAM_ERROR_ "助変数の値は関数（" KEYWORD_FUNCTION "）かクラス（" KEYWORD_OBJECT "）として上げなくては駄目、上がったのは「{0}」"
#define _FAILURE_LENGTH_ "値は長さがない"
#define _FAILURE_SIZE_ "値は大きさがない"
#define _FAILURE_EXTEND_ "「static」で出来たクラス（" KEYWORD_OBJECT "）を土台として使うのは駄目"
#define _FAILURE_STR_TO_NUM_ "文字列（" KEYWORD_STRING "）の「{0}」は数にするのは駄目"
#define _FAILURE_CONVERT_ "値の変換は駄目"
#define _FAILURE_ALLOC_ "ゼロ以下の長さの同意列（" KEYWORD_ARRAY "）は駄目"
#define _FAILURE_TO_STR_ "値は文字列の変換は駄目"
#define _FAILURE_STRING_FORMAT_ "「文字列」（" KEYWORD_STRING "）の解析についてまだ部分が残っている"
#define _NOT_NUMBER_ "値の種類は「数」（" KEYWORD_NUMBER "）ではない"
#define _NOT_POINTER_ "値の種類は「ポインター」（" KEYWORD_POINTER "）ではない"
#define _NOT_DICTIONARY_ "値の種類は「テーブル」（" KEYWORD_DICTIONARY "）ではない"
#define _NOT_VECTOR_ "値の種類は「同意列」（" KEYWORD_ARRAY "）ではない"
#define _NOT_STRING_ "値の種類は「文字列」（" KEYWORD_STRING "）ではない"
#define _NOT_BOOLEAN_ "値の種類は「ブール」（" KEYWORD_BOOLEAN "）ではない"
#define _NOT_OBJECT_ "値の種類は「クラス」（" KEYWORD_OBJECT "）ではない"
#define _NOT_TYPE_ "値の種類は「種類」（" KEYWORD_TYPE "）ではない"
#define _NOT_FUNCTION_ "値の種類は「関数」（" KEYWORD_FUNCTION "）ではない"
#define _FUNCTION_VALUE_NOT_EXIST_ "くれた種類の関数（" KEYWORD_FUNCTION "）は不明"
#define _FUNCTION_ARG_SIZE_FAILURE_ "助変数の大きさの過負荷は関数（" KEYWORD_FUNCTION "）に不明"
#define _INDEX_OUT_OF_BOUNDS_ "指数は遠すぎる：大きさは「{0}」、「{1}」を上げた"
#define _FAILURE_PARSE_CODE_ "コードの解析は失敗"
#define _EXPECTED_ERROR_ "「{0}」は思い設けられた"
#define _EXPECTED_FUNCTION_PARAM_ "関数（" KEYWORD_FUNCTION "）の助変数は思い設けられた"
#define _EXPECTED_BASE_TYPE_ "関数（" KEYWORD_FUNCTION "）の宣言は基本的な種類を思い設けた"
#define _EXPECTED_FUNCTION_NAME_ "関数（" KEYWORD_FUNCTION "）の名は思い設けられた"
#define _EXPECTED_FUNCTION_SIG_ "関数（" KEYWORD_FUNCTION "）の署名は思い設けられた"
#define _EXPECTED_OBJECT_NAME_ "クラス（" KEYWORD_OBJECT "）の識別名は思い設けられた"
#define _EXPECTED_BASE_CAST_ "キャストするために基本的な種類は思い設けられた"
#define _EXPECTED_RH_ "右に表現は思い設けられた"
#define _UNEXPECTED_TERMINATION_ "案外に表現が終わった"
#define _NOT_UNARY_OP_ "一項の作動で演算子をするのは駄目"
#define _VALUE_KEY_PAIR_ERROR_ "キーと値の組を作るために使うのは駄目"
#define _EXPECTED_AFTER_ "「{0}」の後で表現は思い設けられた"
#define _EXPECTED_IDF_ "識別名は思い設けられた"
#define _EXPECTED_EXPR_ "表現は思い設けられた"
#define _EXPECTED_FILE_ "「" KEYWORD_LOAD "」の後でファイルの道は思い設けられた"
#define _UNDECLARED_OPERATOR_ERROR_ "作動の「{0}」はくれた種類に不明"
#define _STACK_TRACE_MORE_ " ... （これから{0}以上） ..."
#define _EXTERNAL_LIBRARY_NOT_EXIST_ "外付けのライブラリーはない：「{0}」"
#define _EXPORT_FUNCTION_NOT_FOUND_ "ライブラリーの中に輸出の関数はない：「{0}」"
#define _LIBRARY_NOT_IN_MEMORY_ "メモリーにライブラリーはロードされなかった：「{0}」`"
#define _LIBRARY_FUNCTION_NOT_EXIST_ "「{0}」のライブラリーは関数を輸出しない：「{1}」"
#define _INCOMPATIBLE_VECTOR_SIZES_ "同意列（" KEYWORD_ARRAY "）の値の大きさは相容れない"
#define _CANNOT_MAKE_CONST_ "表現は一定の法に解析されない"
#endif

#endif