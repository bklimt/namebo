
import * as functions from 'firebase-functions';

import { generate } from './generate';

export const word = functions.https.onCall((data) => {
  return generate();
});
