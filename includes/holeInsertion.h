/* 
 * Copyright 2021 Philipp Mayer - philmay1992@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "triangulation.h"
#include "triangle.h"
#include "vertex.h"
#include "tedge.h"
#include "tpolygon.h"
#include "settings.h"

/*
	The function insertHole() inserts one hole into an inner triangle of the
	polygon. The inner triangle is selected randomly based on triangle specific
	weights. The inserted hole has the same shape as the triangle into which
	it is inserted, but is shrunk by 50% in each direction. The area in the
	triangle, but external to the hole gets retriangulated.

	@param 	T 	The triangulation the polygon lives in
*/
void insertHole(Triangulation *T);
