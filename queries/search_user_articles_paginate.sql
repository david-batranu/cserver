SELECT uri,
       title,
       pubdate
FROM Articles
WHERE title LIKE '%:SearchString%'
  AND id NOT IN
    (SELECT id
     FROM Articles
     WHERE sourceid IN
         (SELECT sourceid
          FROM UserSources
          WHERE userid = :UserID)
     ORDER BY -pubdate
     LIMIT :PageOffset)
  AND sourceid IN
    (SELECT sourceid
     FROM UserSources
     WHERE userid = :UserID)
ORDER BY -pubdate
LIMIT :PageSize;

