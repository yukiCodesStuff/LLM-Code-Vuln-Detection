            {
            *errorcodeptr = ERR48;
            goto FAILED;
            }

          /* Count named back references. */

          if (!is_recurse) cd->namedrefcount++;

          /* We have to allow for a named reference to a duplicated name (this
          cannot be determined until the second pass). This needs an extra
          16-bit data item. */

          *lengthptr += IMM2_SIZE;

          /* If this is a forward reference and we are within a (?|...) group,
          the reference may end up as the number of a group which we are
          currently inside, that is, it could be a recursive reference. In the
          real compile this will be picked up and the reference wrapped with
          OP_ONCE to make it atomic, so we must space in case this occurs. */

          /* In fact, this can happen for a non-forward reference because
          another group with the same number might be created later. This
          issue is fixed "properly" in PCRE2. As PCRE1 is now in maintenance
          only mode, we finesse the bug by allowing more memory always. */

          *lengthptr += 2 + 2*LINK_SIZE;

          /* It is even worse than that. The current reference may be to an
          existing named group with a different number (so apparently not
          recursive) but which later on is also attached to a group with the
          current number. This can only happen if $(| has been previous
          encountered. In that case, we allow yet more memory, just in case.
          (Again, this is fixed "properly" in PCRE2. */

          if (cd->dupgroups) *lengthptr += 4 + 4*LINK_SIZE;

          /* Otherwise, check for recursion here. The name table does not exist
          in the first pass; instead we must scan the list of names encountered
          so far in order to get the number. If the name is not found, leave
          the value of recno as 0 for a forward reference. */

          else
            {
            ng = cd->named_groups;
            for (i = 0; i < cd->names_found; i++, ng++)
              {
              if (namelen == ng->length &&
                  STRNCMP_UC_UC(name, ng->name, namelen) == 0)
                {
                open_capitem *oc;
                recno = ng->number;
                if (is_recurse) break;
                for (oc = cd->open_caps; oc != NULL; oc = oc->next)
                  {
                  if (oc->number == recno)
                    {
                    oc->flag = TRUE;
                    break;
                    }
                  }
                }
              }
            }