# Naming Conventions

This is my proposal for naming:

* `static` objects use snake_case
* `struct` and `virtual` objects use UpperCamelCase
* Variable names use lowerCamelCase
* Functions, being themselves variables, also use lowerCamelCase
* Constant values and enum values CAPITAL_SNAKE_CASE
* Filenames for files containing one object or one function reflect that object or function in the name
* Filenames that contain multiple objects or multiple functions use snake_case as a form of 'namespacing' to reflect the contents
* Things prefixed with `_` should be reserved for library or system use.