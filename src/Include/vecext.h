#pragma once

#include "pch.h"

//! gKit vec.h extensions

bool equal(float a, float b);

//! return true if a < b;
bool operator< ( const Vector& a, const Vector& b );
//! return true if a > b;
bool operator> ( const Vector& a, const Vector& b );
//! return true if a == b;
bool operator== ( const Vector& a, const Vector& b );

Vector operator+= ( const Vector& a, const Vector& b );
Vector operator*= ( const Vector& a, const Vector& b );

Vector min( const Vector& a, const Vector& b );
Vector max( const Vector& a, const Vector& b );
