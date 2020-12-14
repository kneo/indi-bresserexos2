/*
 * SerialCommand.hpp
 * 
 * Copyright 2020 Kevin Krüger <kkevin@gmx.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
#ifndef _CIRCULARBUFFER_H_INCLUDED_
#define _CIRCULARBUFFER_H_INCLUDED_

#include <cstdint>
#include "Config.hpp"

namespace
{
	template<typename T,int max_size>
	class CircularBuffer
	{
		public:
			CircularBuffer()
			{
				
			}
			
			virtual ~CircularBuffer()
			{
				
			}
			
			void push_back()
			{
				
			}
			
			T pop_front()
			{
				
			}
			
			T front()
			{
				
			}
			
			size_t size()
			{
				return 0;
			}
			
			
		private:
			T buffer[max_size];
	}
}

#endif
