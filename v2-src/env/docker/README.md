# docker image about guatos's compiler env.

## build or pull image

1. you can build this docker by dockerfile and blow command.

```
//docker build -t YOUR_IMAGE_NAME .

//example:
docker build -t jieshicheng/guatos .
```

2. os just use the image that I uploaded in dockerhub by blow command.

```
docker pull jieshicheng/guatos
```

## run image

```
//docker run -it --name guatos_compiler -v YOUR_GUATOS_PATH:/root/guatos/ jieshicheng/guatos

//example:
docker run -it --name guatos_compiler -v /Users/jieshi/Documents/GitHub/guatOS/:/root/repo/guatOS jieshicheng/guatos
```
