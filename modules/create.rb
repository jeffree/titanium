#!/usr/bin/env ruby
#
# script for creating a template module that can
# allow you to get started really fast with basic
# files pre-created
#
#
require 'fileutils'

dir = File.expand_path(File.dirname(__FILE__))

arg = ARGV.first
if arg.nil?
  $stderr.puts "Usage: create.rb <module_name>"
  exit 1
end

name = arg.downcase

if name=~/^ti\./
  s = name.index '.'
  name = name[s+1..-1]
end

module_name = name.capitalize
module_dir_name = "ti.#{module_name}"
header_define = name.upcase


FileUtils.mkdir_p module_dir_name unless File.exists?(module_dir_name)

bh = File.open(File.join(module_dir_name,"#{name}_binding.h"),'w')
bh.puts <<-END
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _#{header_define}_BINDING_H_
#define _#{header_define}_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace ti
{
	class #{module_name}Binding : public StaticBoundObject
	{
	public:
		#{module_name}Binding(BoundObject*);
	protected:
		virtual ~#{module_name}Binding();
	private:
		BoundObject *global;
	};
}

#endif
END
bh.close


bc = File.open(File.join(module_dir_name,"#{name}_binding.cpp"),'w')
bc.puts <<-END
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "#{name}_binding.h"

namespace ti
{
	#{module_name}Binding::#{module_name}Binding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
	}
	#{module_name}Binding::~#{module_name}Binding()
	{
		KR_DECREF(global);
	}
}
END
bc.close

mh = File.open(File.join(module_dir_name,"#{name}_module.h"),'w')
mh.puts <<-END
/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef TI_#{header_define}_MODULE_H_
#define TI_#{header_define}_MODULE_H_

#include <kroll/kroll.h>

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define TITANIUM_API EXPORT
#elif defined(OS_WIN32)
# ifdef TITANIUM_API_EXPORT
#  define TITANIUM_API __declspec(dllexport)
# else
#  define TITANIUM_API __declspec(dllimport)
# endif
# define EXPORT __declspec(dllexport)
#endif

namespace ti 
{
	class TITANIUM_API #{module_name}Module : public kroll::Module
	{
		KROLL_MODULE_CLASS(#{module_name}Module)
		
		void Test();

	private:
		kroll::BoundObject *variables;
	};

}
#endif
END
mh.close

mc = File.open(File.join(module_dir_name,"#{name}_module.cpp"),'w')
mc.puts <<-END
/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "#{name}_module.h"
#include "#{name}_test.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(#{module_name}Module);
	
	void #{module_name}Module::Initialize()
	{
		// load our variables
		this->variables = new #{module_name}Binding(host->GetGlobalObject());

		// set our #{module_dir_name}
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("File",value);
		KR_DECREF(value);
	}

	void #{module_name}Module::Destroy()
	{
		KR_DECREF(this->variables);
	}
	
	void #{module_name}Module::Test()
	{
	  #{module_name}UnitTestSuite test;
	  test.Run(host);
	}
}
END
mc.close

th = File.open(File.join(module_dir_name,"#{name}_test.h"),'w')
th.puts <<-END
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_#{header_define}_TEST_H_
#define TI_#{header_define}_TEST_H_
#include "#{name}_test.h"
#include "#{name}_module.h"
#include "#{name}_binding.h"

namespace ti
{
	class #{module_name}UnitTestSuite
	{
    public:
      #{module_name}UnitTestSuite() {};
      virtual ~#{module_name}UnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
END
th.close

tc = File.open(File.join(module_dir_name,"#{name}_test.cpp"),'w')
tc.puts <<-END
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "#{name}_test.h"
#include "#{name}_module.h"
#include "#{name}_binding.h"

namespace ti
{
	void #{module_name}UnitTestSuite::Run(Host *host)
	{
	  //TODO: write your test cases here
	}
}
END
tc.close

sc = File.open(File.join(module_dir_name,"SConscript"),'w')
sc.puts <<-END
#!/usr/bin/env python
import os

Import("tiBuild")
Import('build')

env = tiBuild.env.Clone();
env.Append(CPPDEFINES = {'TITANIUM_API_EXPORT' : 1})
env.Append(CPPPATH = ['#kroll'])


s = env.SharedLibrary(target = tiBuild.dir + '/ti#{name}module', source = Glob('*.cpp'))
END
sc.close



puts "Created: #{module_dir_name}"
exit 0
