UPDATE Users 
SET info = SUBSTRING(LoginStr, 1, 2) 
WHERE info is NULL;
