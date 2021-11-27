/*
    This file is part of CADauno.
    Copyright (C) 2009 Giampaolo Capelli

    CADauno is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    CADauno is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CADauno; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SERIALIZATOR
#define SERIALIZATOR

void load_points(int id);
void save_points(int id);

class Serializator
{
public:
	virtual void init_points() = 0;
	virtual void load_points(int id) = 0;
	virtual void save_points(int id) = 0;
};

class DefaultSerializator : public Serializator
{
public:
	DefaultSerializator();
	void init_points();
	void load_points(int id);
	void save_points(int id);
};

class SerializatorFactory
{
	static Serializator *ser;

public:
	static Serializator *getSerializator()
	{
		if (SerializatorFactory::ser == NULL)
		{

			//to use different serializator,
			//change this assignment usign a new instance
			//of the wanted serializator class
			SerializatorFactory::ser = new DefaultSerializator();
		}
		return ser;
	}
};

#endif