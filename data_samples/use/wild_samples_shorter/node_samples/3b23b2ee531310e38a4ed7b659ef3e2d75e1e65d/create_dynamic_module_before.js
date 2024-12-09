/**
 * Creates an export for a given module.
 * @param {string} expt - The name of the export.
 */
function createExport(expt) {
  const name = `${expt}`;
  return `let $${name};
export { $${name} as ${name} };
import.meta.exports.${name} = {
  get: () => $${name},
  set: (v) => $${name} = v,
};`;
}

/**