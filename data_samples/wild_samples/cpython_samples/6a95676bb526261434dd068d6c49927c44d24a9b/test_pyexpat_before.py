        def resolve_entity(context, base, system_id, public_id):
            handler_call_args.append((public_id, system_id))
            return 1

        parser = expat.ParserCreate()
        parser.UseForeignDTD(True)
        parser.SetParamEntityParsing(expat.XML_PARAM_ENTITY_PARSING_ALWAYS)
        parser.ExternalEntityRefHandler = resolve_entity
        parser.Parse(
            b"<?xml version='1.0'?><!DOCTYPE foo PUBLIC 'bar' 'baz'><element/>")
        self.assertEqual(handler_call_args, [("bar", "baz")])


if __name__ == "__main__":
    unittest.main()