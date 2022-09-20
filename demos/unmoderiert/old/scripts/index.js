window.onload = async function() {
  var inputVariable = document.getElementById('inputVariable');
  var updateVariableButton = document.getElementById('buttonUpdateVariable');

  buttonUpdateVariable.addEventListener('click', onButtonUpdateVariableClick);
};

async function onButtonUpdateVariableClick() {
  let enteredVariableValue = inputVariable.value;
  console.log(enteredVariableValue);
};
