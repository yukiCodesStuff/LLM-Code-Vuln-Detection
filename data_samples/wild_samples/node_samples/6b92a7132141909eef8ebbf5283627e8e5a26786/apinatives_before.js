function CreateDate(time) {
  var date = new $Date();
  date.setTime(time);
  return date;
}


var kApiFunctionCache = {};
var functionCache = kApiFunctionCache;


function Instantiate(data, name) {
  if (!%IsTemplate(data)) return data;
  var tag = %GetTemplateField(data, kApiTagOffset);
  switch (tag) {
    case kFunctionTag:
      return InstantiateFunction(data, name);
    case kNewObjectTag:
      var Constructor = %GetTemplateField(data, kApiConstructorOffset);
      // Note: Do not directly use a function template as a condition, our
      // internal ToBoolean doesn't handle that!
      var result = typeof Constructor === 'undefined' ?
          {} : new (Instantiate(Constructor))();
      ConfigureTemplateInstance(result, data);
      result = %ToFastProperties(result);
      return result;
    default:
      throw 'Unknown API tag <' + tag + '>';
  }
}