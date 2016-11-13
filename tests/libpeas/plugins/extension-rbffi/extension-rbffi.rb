#
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


require 'gir_ffi'

GirFFI.setup(:Peas)
GirFFI.setup(:Introspection)

def install_vfunc(info, func)
    fn = lambda { |*args| return args[0].send(func, *args[1..-1]) }
    info.install_vfunc_implementation(func, fn)
end

class ExtensionRbFFI < GObject::Object
    include Introspection::Callable
    # FIXME: HasPrerequisite must come before Base which seems backwards
    include Introspection::HasPrerequisite
    include Introspection::Base
    include Peas::Activatable

    GirFFI.define_type(ExtensionRbFFI) do |info|
        info.g_name = "ExtensionRbFFI"
        #~ bits = GObject::ParamFlags[:readable] |
                #~ GObject::ParamFlags[:writable] |
                #~ GObject::ParamFlags[:construct_only]
        #~ info.install_property GObject.param_spec_boxed("plugin-info",
                                                       #~ "plugin-info",
                                                       #~ "plugin-info",
                                                       #~ Peas::PluginInfo.gtype,
                                                       #~ bits)
                                                       
        bits = GObject::ParamFlags[:readable] |
                GObject::ParamFlags[:writable]
        info.install_property GObject.param_spec_object("object",
                                                        "object",
                                                        "object",
                                                        GObject::Object.gtype,
                                                        bits)

        bits = GObject::ParamFlags[:readable]
        info.install_property GObject.param_spec_int("update-count",
                                                     "update-count",
                                                     "update-count",
                                                     0, 1000000, 0,
                                                     bits)

        # Peas::Activatable vfuncs
        install_vfunc(info, :activate)
        install_vfunc(info, :deactivate)
        install_vfunc(info, :update_state)
        # Introspection::Base vfuncs
        install_vfunc(info, :get_plugin_info)
        install_vfunc(info, :get_settings)
        # Introspection::Callable vfuncs
        install_vfunc(info, :call_no_args)
        install_vfunc(info, :call_with_return)
        install_vfunc(info, :call_single_arg)
        install_vfunc(info, :call_multi_args)
    end

    #~ def self.allocate
        #~ x = super
        #~ x
    #~ end

    def initialize
        super
        puts "initialize #{self.object_id}"
    end

    def activate
    end

    def disable
    end

    def update_state
        @update_count += 1
    end

    def get_plugin_info
        # to_ptr shouldn't be needed?
        return plugin_info.to_ptr
    end

    def get_settings
        plugin_info.get_settings(nil)
    end

    def call_no_args
    end

    def call_with_return
        "Hello, World!"
    end

    def call_single_arg
        true
    end

    def call_multi_args(in_, inout)
        [inout, in_]
    end
end
