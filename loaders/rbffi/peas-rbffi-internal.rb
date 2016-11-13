# -*- coding: utf-8 -*-

#  Copyright (C) 2016 - Thomas Martitz
#
# libpeas is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# libpeas is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

require 'rubygems'
require 'gir_ffi'

class RbffiLoader

    def load(fn, dirname, modname)

        # append module path to Ruby's search paths and require() the file
        $:.unshift(dirname) unless ($:.include?(dirname))
        require(modname + ".rb")
        # look for modules or classes that can be derived from the module name
        # "-" is stripped (not valid in a Ruby identifier) and then
        # case-insensitive comparison is performed
        modname = modname.gsub("-", "")
        c = Module.constants.detect do |c|
            modname.casecmp(c.to_s) == 0 && Module.const_get(c).is_a?(Module)
        end
        c ? Module.const_get(c) : nil
    end

    def implements_interface?(cls, gtype)

        cls.included_modules.each do |mod|
            if (mod.respond_to?(:gtype))
                if (mod.gtype.to_i == gtype)
                    return true
                end
            end
        end

        return false
    end

    def locate_extension(mod, gtype)
        # mod is the Module returned by load(). If it's a Class, use
        # it directly. Otherwise locate the matching class inside mod.
        if (mod.is_a?(Class)) then
            if (implements_interface?(mod, gtype))
                return mod
            end
        else
            # module containing one or more classes
            mod.constants.each do |c|
                klass = mod.const_get(c)
                if (implements_interface?(klass, gtype))
                    return klass
                end
            end
        end
    end

    def find_extension_type(mod, gtype)
        if (klass = locate_extension(mod, gtype))
            return klass.gtype.to_i
        end
    end

    def instantiate(mod, plugin_info)
        obj = mod.class_eval { allocate }
        # convert native pointer to GirFFI object and bind to obj
        plugin_info_mod = Module.const_get(:Peas).const_get(:PluginInfo)
        info = plugin_info_mod.wrap(FFI::Pointer.new(plugin_info))
        obj.instance_variable_set(:@plugin_info, info)
        unless obj.respond_to?(:plugin_info)
            mod.class_eval do 
                attr_reader :plugin_info
            end
        end
        obj.send(:initialize)

        return obj
    end

    def create_extension(mod, gtype, plugin_info)
        if (klass = locate_extension(mod, gtype))
            obj = instantiate(klass, plugin_info)
            return obj
        end
    end
end

# this is so that C code that eval()s this script gets a fresh instance
RbffiLoader.new()
