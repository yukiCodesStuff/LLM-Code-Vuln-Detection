          so far in order to get the number. If the name is not found, leave
          the value of recno as 0 for a forward reference. */

          else
            {
            ng = cd->named_groups;
            for (i = 0; i < cd->names_found; i++, ng++)
              {