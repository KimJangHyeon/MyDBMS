USE Pokemon;
/*1*/
SELECT tr.name From Trainer as tr Join CatchedPokemon as cp On tr.id = cp.owner_id group by tr.name having count(*) > 2 order by count(*) desc;

/*2*/
Select p1.name From Pokemon as p1 Join (Select type from Pokemon as p2 group by type order by count(*)  desc limit 2)p2 On p1.type = p2.type order by p1.name asc;

/*3*/
Select name From Pokemon where name Like '_o%' order by name asc;

/*4*/
Select nickname From CatchedPokemon Where level >= 50 order by nickname asc;

/*5*/
Select name From Pokemon Where  name Like '______' order by name asc;

/*6*/
Select tr.id, tr.name, tr.hometown from Trainer  as tr Join City as ci On tr.hometown = ci.name where ci.name = 'Blue City';

/*7*/
Select distinct tr.hometown from Trainer as tr;

/*8*/
Select Avg(cp.level) from Trainer  as tr Join CatchedPokemon as cp On tr.id = cp.owner_id where tr.name like 'Red';

/*9*/
Select nickname from CatchedPokemon Where nickname not like 'T%' order by nickname asc;

/*10*/
Select nickname From CatchedPokemon Where level >= 50 and owner_id >= 6 order by nickname asc;    

/*11*/
Select id, name  From Pokemon  order by id asc;

/*12*/
SELECT nickname from CatchedPokemon Where level <= 50 order by level asc;

/*13*/
SELECT p.name, p.id FROM Trainer as tr Join CatchedPokemon as cp On tr.id = cp.owner_id Join Pokemon as p On p.id = cp.pid where tr.hometown = 'Sangnok City' order by p.id asc;

/*14*/
Select cp.nickname  From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id Join Pokemon as p On p.id = cp.pid where p.type = 'Water' and tr.id in ( select leader_id From Gym );

/*15*/
Select count(*) from CatchedPokemon as cp where pid in ( select before_id  from Evolution );

/*16*/
Select count(*) From Pokemon Where type in ('Water', 'Electric', 'Psychic');

/*17*/
SELECT p.type, count(*) From Pokemon as p join CatchedPokemon as cp On cp.pid = p.id Join Trainer as tr On tr.id = cp.owner_id where tr.hometown = 'Sangnok City' Group by p.type;

/*18*/
Select cp.level From Trainer as tr Join CatchedPokemon as cp  On cp.owner_id = tr.id where tr.hometown = 'Sangnok City' order by cp.level desc limit 1;

/*19*/
select p.type, count(*) from Pokemon as p Join CatchedPokemon as cp On cp.pid = p.id where cp.owner_id In (select leader_id from Gym where Gym.city = 'Sangnok City') group by type;

/*20*/
Select tr.name, count(*)  From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id group by tr.name  order by count(*) asc;

/*21*/
Select name From Pokemon Where name like 'A%' or name like 'I%' or name like 'E%' or name like 'O%' or name Like 'U%';

/*22*/
Select type, count(*) from Pokemon group by type order by count(*) asc, type asc;

/*23*/
Select distinct tr.name From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id where cp.level <= 10;

/*24*/
Select tr.hometown, Avg(cp.level) From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id Group by tr.hometown order by avg(cp.level) asc;

/*25*/
select Distinct p1.name From Trainer as tr Join CatchedPokemon as cp On tr.id = cp.owner_id Join Pokemon as p1  On p1.id = cp.pid where tr.hometown = 'Sangnok City' and p1.name In ( Select p2.name From Trainer as tr1 Join CatchedPokemon As cp1 On tr1.id = cp1.owner_id Join Pokemon as p2 On p2.id = cp1.pid Where tr1.hometown = 'Brown City');

/*26*/
Select nickname From CatchedPokemon Where nickname like '% %' order by nickname desc;

/*27*/
SELECT MAX(level) From CatchedPokemon as cp join Trainer as tr on cp.owner_id = tr.id Group by owner_id having count(*) >= 4 order by tr.name asc;

/*28*/
Select tr.name, avg(cp.level) from Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id Join Pokemon as p On p.id = cp.pid where p.type = 'Normal' or p.type = 'Electric' group by tr.name order by avg(cp.level) asc;

/*29*/
select p.name, tr.name, c.description From Pokemon as p Join CatchedPokemon as cp  On p.id = cp.pid Join Trainer as tr On tr.id = cp.owner_id Join City as c On c.name = tr.hometown where p.id = 152 order by cp.level desc;

/*30*/
Select p0.id, p0.name, p1.name, p2.name From Pokemon as p0 Join Evolution as e1 On p0.id = e1.before_id Join Pokemon as p1 On p1.id = e1.after_id Join Evolution as e2 On p1.id = e2.before_id Join Pokemon as p2 On p2.id = e2.after_id;

/*31*/
select name from Pokemon where id > 9 and id <100 order by name asc;

/*32*/
select p.name From Pokemon as p  Where p.id not in ( Select cp.pid From CatchedPokemon as cp );

/*33*/
Select Sum(cp.level)  From CatchedPokemon as cp Join Trainer as tr On cp.owner_id = tr.id Where tr.name = 'Matis';

/*34*/
Select tr.name  From Trainer as tr Join Gym as g On g.leader_id = tr.id;

/*35*/
select Pokemon.type, p1.c / count(*) * 100 from Pokemon, (select type, count(*) as c from Pokemon group by type order by c desc limit 1)p1;

/*36*/
Select tr.name From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id Where cp.nickname Like 'A%' Group by tr.name Order by tr.name asc;

/*37*/
Select tr.name, Sum(cp.level)  From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id order by Sum(cp.level) desc Limit 1;

/*38*/
select name From Pokemon where id in (select after_id from Evolution Where after_id not in ( select e2.after_id From Evolution as e1 Join Evolution as e2 On e1.after_id = e2.before_id )) order by name asc;

/*39*/
select distinct tr.name From Trainer as tr Join CatchedPokemon as cp On cp.owner_id = tr.id group by pid having count(*) >= 2 order by tr.name asc;

/*40*/
Select t1.hometown, t1.nickname, t1.level
from City as c
Join (
        Select tr.hometown, cp.nickname, cp.level
        From Trainer as tr
        Join CatchedPokemon as cp
        On cp.owner_id = tr.id
        where tr.hometown = c.name
        order by cp.level desc
        limit 1
        )t1
On t1.hometown = c.name;
