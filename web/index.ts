
import { generate } from './generate.js'

document.addEventListener('DOMContentLoaded', () => {
    var word = document.getElementById("word") as HTMLElement;
    var next = document.getElementById("next") as HTMLButtonElement;

    var genWord = function () {
        next.disabled = true;
        word.innerHTML = generate();
        next.disabled = false;
    };

    next.onclick = genWord;

    genWord();
});
