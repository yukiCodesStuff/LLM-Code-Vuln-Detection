def debug_build(program):
    program = os.path.basename(program)
    name = os.path.splitext(program)[0]
    return name.endswith("_d")


def remove_python_envvars():
    env = dict(os.environ)
            if expected['stdio_errors'] is self.GET_DEFAULT_CONFIG:
                expected['stdio_errors'] = 'surrogateescape'

        if sys.platform == 'win32':
            default_executable = self.test_exe
        elif expected['program_name'] is not self.GET_DEFAULT_CONFIG:
            default_executable = os.path.abspath(expected['program_name'])
        else:
        pre_config = dict(configs['pre_config'])
        for key, value in list(expected.items()):
            if value is self.IGNORE_CONFIG:
                del pre_config[key]
                del expected[key]
        self.assertEqual(pre_config, expected)

    def check_config(self, configs, expected):
        config = dict(configs['config'])
        for key, value in list(expected.items()):
            if value is self.IGNORE_CONFIG:
                del config[key]
                del expected[key]
        self.assertEqual(config, expected)

    def check_global_config(self, configs):
        self.check_pre_config(configs, expected_preconfig)
        self.check_config(configs, expected_config)
        self.check_global_config(configs)

    def test_init_default_config(self):
        self.check_all_configs("test_init_initialize_config", api=API_COMPAT)

        }
        self.default_program_name(config)
        env = {'TESTPATH': os.path.pathsep.join(paths)}
        self.check_all_configs("test_init_setpath", config,
                               api=API_COMPAT, env=env,
                               ignore_stderr=True)

                # Copy pythonXY.dll (or pythonXY_d.dll)
                ver = sys.version_info
                dll = f'python{ver.major}{ver.minor}'
                if debug_build(sys.executable):
                    dll += '_d'
                dll += '.dll'
                dll = os.path.join(os.path.dirname(self.test_exe), dll)
                dll_copy = os.path.join(tmpdir, os.path.basename(dll))
                shutil.copyfile(dll, dll_copy)

            # Copy Python program
            exec_copy = os.path.join(tmpdir, os.path.basename(self.test_exe))
            shutil.copyfile(self.test_exe, exec_copy)
                config['base_prefix'] = pyvenv_home
                config['prefix'] = pyvenv_home
            env = self.copy_paths_by_env(config)
            self.check_all_configs("test_init_compat_config", config,
                                   api=API_COMPAT, env=env,
                                   ignore_stderr=True, cwd=tmpdir)

    def test_global_pathconfig(self):
        # Test C API functions getting the path configuration:
        #