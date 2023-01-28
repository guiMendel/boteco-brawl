#!/usr/bin/env bash
# This file automatically goes through all files under the include folder and, for all files which match some given patterns, writes CPP files with functions to generate table entries for each of those files

# Right now, it only finds component scripts and writes the component table file

echo "Generating component table assembler..."

# Regex for a C label
CLabel="[a-zA-Z_][a-zA-Z0-9_]*"

# Start off with the template file
AssemblerScript=$(cat ./src/editor/componentTable/template.cpp)

# Add disclaimer to script
AssemblerScript="// DO NOT MANUALLY EDIT THIS FILE. IT WAS AUTOMATICALLY GENERATED BY THE SCRIPT $(basename "$0")\n${AssemblerScript}"

HeaderInit='/* COMPONENT CLASS INCLUDES */'
EntriesInit='/* ADD COMPONENTS TO TABLE */'

AfterHeader=${AssemblerScript#*$HeaderInit}
AfterEntries=${AssemblerScript#*$EntriesInit}

# Index where to start adding new include headers
HeaderIndex=$(( ${#AssemblerScript} - ${#AfterHeader} ))

# Index where to start adding new entries
EntriesIndex=$(( ${#AssemblerScript} - ${#AfterEntries} ))

# Given a component class name, adds the include header to the assembler script
# $1 must contain class name
function include_header_for() {
  Class=$1

  # String to add to script
  NewInclude="\n#include <$Class.h>"

  AssemblerScript="${AssemblerScript:0:HeaderIndex}$NewInclude${AssemblerScript:HeaderIndex}"

  # Update header index AND entry index
  HeaderIndex=$(( ${HeaderIndex} + ${#NewInclude}))
  EntriesIndex=$(( ${EntriesIndex} + ${#NewInclude}))
}

# Given a component class name, adds the table entry line to the assembler script
# $1 must contain component path
# $2 must contain class name
function table_entry_for() {
  ComponentPath=$1
  Class=$2

  # Which types are allowed to be parsed (should be the same listed in the ComponentParameterType)
  AllowedTypesRegex="(int|float|(std::)?string|bool)"

  # First, find all constructors of this component (with primitive types only)
  ConstructorsRegex="$Class*\s*\(\s*WorldObject\s*&\s*$CLabel\s*(,\s*(const\s+)?$AllowedTypesRegex\s*$CLabel\s*(=\s*.+\s*)?)*\)"
  # We will use the FIRST match
  Constructor=$(cat "$ComponentPath" | grep -oP "$ConstructorsRegex")

  # First part of string to add
  NewEntry="\n\n  // $Class
  {
    // Specify parameters"

  # Get all parameters (except first one) of the selected constructor as strings in the format (type name)
  ParamsRegex=",\s*(const\s+)?$AllowedTypesRegex\s*[a-zA-Z_][a-zA-Z0-9_]*"

  # For each parameter match
  readarray -t Params <<< $( echo "$Constructor" | grep -oP "$ParamsRegex")
  
  for match in "${Params[@]}"; do
    # Get type
    Type=$( echo "$match" | grep -oP "$AllowedTypesRegex")
  
    # Get param name
    Name=$( echo "$match" | grep -oP "$CLabel$")

    # Get type as enum value
    Type=$( echo "$Type" | grep -oP "$CLabel$")
    TypeEnum="${Type^}"

    # Add param line to entry
    NewEntry="$NewEntry
    paramTypes[\"$Name\"] = ComponentParameterType::$TypeEnum;"
  done

  # Add the rest of the string
  NewEntry="$NewEntry

    // Add to table
    table[\"$Class\"] = make_pair(map(paramTypes), MAKE_INSERTER($Class));

    paramTypes.clear();
  }"

  # Add to the script
  AssemblerScript="${AssemblerScript:0:EntriesIndex}$NewEntry${AssemblerScript:EntriesIndex}"

  # Update header index
  EntriesIndex=$(( ${EntriesIndex} + ${#NewEntry}))
}

# Treats component scripts
# $1 must contain script path
# $2 must contain class name
function treat_component() {
  # Alias params
  ComponentPath=$1
  Class=$2

  # Check that this component has the necessary ComponentParameter map constructor to be added to the table
  CosntructorRegex="$Class\s*\(\s*WorldObject\s*&\s*$CLabel\s*,\s*ComponentParameters\s*&\s*$CLabel\s*\)"
  HasConstructor=$(cat $ComponentPath | grep -oP "$CosntructorRegex")

  # Only if that's the case
  if [[ -n "$HasConstructor" ]]
  then
    echo "$Class has constructor"

    # Add the include header
    include_header_for $Class

    # Add the table entry
    table_entry_for $ComponentPath $Class
  fi
}

# For each script file
for Script in ./include/**/*.h; do
  # Detect component scripts

  # Find the component class declaration line
  Class=$(cat $Script | grep -oP "(?<=class )$CLabel(?= : public Component)")

  # If it exists
  if [[ -n "$Class" ]]
  then
    treat_component $Script $Class
  fi
done

# Write table file
printf "$AssemblerScript" > ./src/editor/componentTable/assembler.cpp

echo "Assembler ready"
