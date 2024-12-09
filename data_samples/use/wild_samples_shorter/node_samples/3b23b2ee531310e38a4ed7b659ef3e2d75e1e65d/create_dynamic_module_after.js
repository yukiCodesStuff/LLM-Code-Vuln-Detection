/**
 * Creates an export for a given module.
 * @param {string} expt - The name of the export.
 * @param {number} index - The index of the export statement.
 */
function createExport(expt, index) {
  const nameStringLit = JSONStringify(expt);
  return `let $export_${index};
export { $export_${index} as ${nameStringLit} };
import.meta.exports[${nameStringLit}] = {
  get: () => $export_${index},
  set: (v) => $export_${index} = v,
};`;
}

/**